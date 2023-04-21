#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

#include <cstdlib>
#include <unistd.h>
#include <openssl/sha.h>

#include "common/Defaults.hpp"
#include "common/Errors.hpp"
#include "common/Helpers.hpp"
#include "common/Logger.hpp"
#include "peer/Leecher.hpp"
#include "peer/Seeder.hpp"
#include "StartParams.hpp"

namespace 
{
    struct BadOption : BT::CException 
    {
        BadOption (std::string const& option)
            : BT::CException("Bad option: " + option,
                             "Use -h for help.") 
        {}
    };

    struct PeerLimitExceeded : BT::CException 
    {
        PeerLimitExceeded(int const peerLimit)
            : BT::CException("Exceeded peer limit.",
                             "Max number of peers = " + std::to_string(peerLimit))
        {}
    };

    struct TorrentFileMissing : BT::CException 
    {
        TorrentFileMissing ()
            : BT::CException("Torrent file missing.",
                             "Use -h for help.") 
        {}
    };

    struct InvalidPeerExpression : BT::CException 
    {
        InvalidPeerExpression(std::string const& peerStr) 
            : BT::CException("Incorrect Peer description: " + peerStr,
                             "Specify the peer using ip:port format.")
        {}
    };

    struct InvalidIPAddress : BT::CException 
    {
        InvalidIPAddress(std::string const& ipStr) 
            : BT::CException("Invalid Peer's address: " + ipStr,
                             "Peer's address must be a valid IPv4 address.")
        {}
    };

    struct InvalidPortNumber : BT::CException 
    {
        InvalidPortNumber(std::string const& port) 
            : BT::CException("Invalid Peer's port: " + port,
                             "Reserved ports (0-1023) are not accepted. Use ephemeral ports (1024-65535).")
        {}
    };

    bool const isValidIPv4Address(std::string const& str) 
    {
        auto isValidIPv4Octet = [](int const value) 
        {
            int constexpr min_ipv4_octet = 0;
            int constexpr max_ipv4_octet = 255;
            return value >= min_ipv4_octet && value <= max_ipv4_octet;
        };

        char constexpr ipv4_delim = '.';
        size_t constexpr ipv4_octets_count = 4;

        size_t nTokens = 0;
        std::istringstream iss(str);
        std::string token;
        while (std::getline(iss, token, ipv4_delim)) 
        {
            if (!isValidIPv4Octet(std::stoi(token)))
            {
                return false;
            }

            nTokens++;
            if (nTokens > ipv4_octets_count)
            {
                return false;
            }
        }

        return (nTokens == ipv4_octets_count);
    }

    bool const isValidPort(std::string const& str) 
    {
        int constexpr min_ephemeral_port = 1024;
        int constexpr max_ephemeral_port = 65535;
        int const portValue = std::stoi(str);
        return (portValue >= min_ephemeral_port && portValue <= max_ephemeral_port);
    }

    std::vector<std::string> const splitIntoIPAndPort(std::string const& str) 
    {
        char constexpr delim = ':';

        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;
        while (std::getline(iss, token, delim))
        {
            tokens.push_back(token);
        }

        if (tokens.size() != 2)
        {
            throw InvalidPeerExpression(str);
        }

        if (!isValidIPv4Address(tokens[0]))
        {
            throw InvalidIPAddress(tokens[0]);
        }

        if (!isValidPort(tokens[1]))
        {
            throw InvalidPortNumber(tokens[1]);
        }

        return tokens;
    }
}

namespace BT 
{
    std::ostream& operator<<(std::ostream& os, StartParams const& params) 
    {
        os << std::endl;
        os << "*** BitTorrent user configuration ***" << std::endl;
        os << "    .torrent file: " << params.torrentFilename << std::endl;
        os << "    Save file: " << params.saveFilename << std::endl;
        os << "    Log file: " << params.logFilename << std::endl;
        if (params.IsSeeder()) 
        {
            os << "    Seeder's port: " << params.seederPort << std::endl;
        }
        else 
        {
            os << "    Leecher's peers information: " << std::endl;
            for (Peer const& peer : params.peers)
            {
                os << "        " << peer << std::endl;
            }
        }

        return os;
    }

    bool StartParams::IsSeeder() const
    {
        return peers.empty();
    }

    void StartParams::throwException(std::exception const& e) const 
    {
        if (!helpRequested)
        {
            throw e;
        }
    }

    void StartParams::initFlagOptionHandlers() 
    {
        flagOptionHandler.clear();
        flagOptionHandler["-h"] = [&]() { helpRequested = true; };
        flagOptionHandler["-v"] = [&]() { enableVerbose = true; };
    }

    void StartParams::initKeyValueOptionsHandlers() 
    {
        keyValueOptionHandler.clear();
        // TODO: Invalid save and log file names?
        keyValueOptionHandler["-s"] = [&](std::string const& value) { saveFilename = value; };
        keyValueOptionHandler["-l"] = [&](std::string const& value) { logFilename = value; };
        keyValueOptionHandler["-I"] = [&](std::string const& value) { clientId = std::stoi(value); };
        keyValueOptionHandler["-b"] = [&](std::string const& value) 
        { 
            if (!isValidPort(value))
            {
                throwException(InvalidPortNumber(value));
            }
            seederPort = std::stoi(value);
        };
        keyValueOptionHandler["-p"] = [&](std::string const& value)
        {
            if (peers.size() >= Defaults::MaxConnections)
            {
                throwException(PeerLimitExceeded(Defaults::MaxConnections));
            }
            auto tokens = splitIntoIPAndPort(value);
            peers.push_back(Peer(tokens[0], std::stoi(tokens[1])));
        };
    }

    void StartParams::initOptionHandlers() 
    {
        initFlagOptionHandlers();
        initKeyValueOptionsHandlers();
    }

    void StartParams::parseArgs(std::vector<std::string> const& args)
    {
        if (args.size() == 0) 
        {
            helpRequested = true;
            return;
        }

        for (size_t i = 0; i < args.size(); i++)
        {
            std:: string const& arg = args[i];
            if (flagOptionHandler.contains(arg)) 
            {
                flagOptionHandler[arg]();
            }
            else if (keyValueOptionHandler.contains(arg) && 
                     i + 1 < args.size() && args[i+1][0] != '-') 
            {
                keyValueOptionHandler[arg](args[i+1]);
                i++;
            }
            else 
            {
                char constexpr * const torrentFileExt = ".torrent";
                if (arg[0] != '-' &&
                    arg.ends_with(torrentFileExt) && 
                    torrentFilename.empty())
                {
                    torrentFilename = arg;
                    continue;
                }

                throwException(BadOption(args[i]));
            }
        }

        if (torrentFilename.empty())
        {
            throwException(TorrentFileMissing());
        }

        if (saveFilename.empty()) 
        {
            saveFilename = torrentFilename + ".save";
        }
    }

    StartParams::StartParams(/* IN  */ std::vector<std::string> const& args,
                             /* OUT */ STATUSCODE& rStatus)
        : helpRequested(false),
          enableVerbose(false),
          clientId(0),
          seederPort(Defaults::DefaultPort),
          logFilename(Defaults::DefaultLogFilename)
    {
        try
        {
            initOptionHandlers();
            parseArgs(args);
            rStatus = STATUSCODE::SC_SUCCESS;
        }
        catch(std::exception const& e)
        {
            Error(e.what());
            rStatus = STATUSCODE::SC_FAIL_BAD_PARAMETER;
        }
    }
}

