#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>

#include <cstdlib>
#include <unistd.h>
#include <openssl/sha.h>

#include "Config.hpp"
#include "Seeder.hpp"
#include "Leecher.hpp"
#include "Defaults.hpp"
#include "helpers.hpp"
#include "Errors.hpp"

namespace 
{
    std::string const option_help = "-h";
    std::string const option_bindport = "-b";
    std::string const option_savefile = "-s";
    std::string const option_logfile = "-l";
    std::string const option_peers = "-p";
    std::string const option_id = "-I";
    std::string const option_verbose = "-v";

    struct BadOption : BT::BTException 
    {
        BadOption (std::string const& option)
            : BT::BTException("Bad option: " + option,
                              "Use -h for help.") 
        {}
    };

    struct PeerLimitExceeded : BT::BTException 
    {
        PeerLimitExceeded(int const peerLimit)
            : BT::BTException("Exceeded peer limit.",
                              "Max number of peers = " + std::to_string(peerLimit))
        {}
    };

    struct TorrentFileMissing : BT::BTException 
    {
        TorrentFileMissing ()
            : BT::BTException("Torrent file missing.",
                              "Use -h for help.") 
        {}
    };

    struct InvalidPeerExpression : BT::BTException 
    {
        InvalidPeerExpression(std::string const& peerStr) 
            : BT::BTException("Incorrect Peer description: " + peerStr,
                              "Specify the peer using ip:port format.")
        {}
    };

    struct InvalidIPAddress : BT::BTException 
    {
        InvalidIPAddress(std::string const& ipStr) 
            : BT::BTException("Invalid Peer's address: " + ipStr,
                              "Peer's address must be a valid IPv4 address.")
        {}
    };

    struct InvalidPortNumber : BT::BTException 
    {
        InvalidPortNumber(std::string const& port) 
            : BT::BTException("Invalid Peer's port: " + port,
                              "Reserved ports (0-1023) are not accepted. Use ephemeral ports (1024-65535).")
        {}
    };

    bool const isValidIPv4Address(std::string const& str) 
    {
        auto isValidIPv4Octet = [](int const value) 
        {
            int const min_ipv4_octet = 0;
            int const max_ipv4_octet = 255;
            return value >= min_ipv4_octet && value <= max_ipv4_octet;
        };

        char const ipv4_delim = '.';
        int const ipv4_octets_count = 4;

        std::istringstream iss(str);
        std::string token;
        int nTokens = 0;
        while (std::getline(iss, token, ipv4_delim)) 
        {
            if (!isValidIPv4Octet(std::stoi(token)))
                return false;

            nTokens++;
            if (nTokens > ipv4_octets_count)
                return false;
        }

        return (nTokens == ipv4_octets_count);
    }

    bool const isValidPort(std::string const& str) 
    {
        int const min_ephemeral_port = 1024;
        int const max_ephemeral_port = 65535;
        int const portValue = std::stoi(str);
        return (portValue >= min_ephemeral_port && portValue <= max_ephemeral_port);
    }

    std::vector<std::string> const splitIntoIPAndPort(std::string const& str) 
    {
        std::string token;
        std::vector<std::string> tokens;
        std::istringstream iss(str);

        char const delim = ':';
        while (std::getline(iss, token, delim))
            tokens.push_back(token);

        if (tokens.size() != 2)
            throw InvalidPeerExpression(str);

        if (!isValidIPv4Address(tokens[0]))
            throw InvalidIPAddress(tokens[0]);

        if (!isValidPort(tokens[1]))
            throw InvalidPortNumber(tokens[1]);

        return tokens;
    }

    void printDefaults(std::ostream& os) 
    {
        os << "*** BitTorrent application configuration ***" << std::endl;
        os << "    Default log file name: " << BT::Defaults::defaultLogFilename << std::endl;
        os << "    Maximum internal buffer size: " << BT::Defaults::MaxBufferSize << std::endl;
        os << "    Maximum number of peer connections: " << BT::Defaults::MaxConnections << std::endl;
        os << "    Range of ports to use for reaching peers: " << BT::Defaults::InitPort << "-" << BT::Defaults::MaxPort << std::endl;
        os << "    Default port: " << BT::Defaults::DefaultPort << std::endl;
        os << "    Maximum supported size of internal buffers:" << std::endl;
        os << "        File name length: " << BT::Defaults::MaxFilenameSize << std::endl;
        os << "        Block length: " << BT::Defaults::BlockSize << std::endl;
        os << "        IP address size: " << BT::Defaults::IPSize << std::endl;
        os << "        IP port size: " << BT::Defaults::PortSize << std::endl;
        os << "        SHA1 Hash size: " << BT::Defaults::Sha1MdSize << std::endl;
    }    
}

namespace BT 
{
    std::ostream& operator<<(std::ostream& os, Config_t const& config) 
    {
        printDefaults(os);

        os << std::endl;
        os << "*** BitTorrent user configuration ***" << std::endl;
        os << "    .torrent file: " << config.torrentFilename << std::endl;
        os << "    Save file: " << config.saveFilename << std::endl;
        os << "    Log file: " << config.logFilename << std::endl;

        if (config.isSeeder()) 
        {
            os << "    Seeder's port: " << config.seederPort << std::endl;
        }
        else 
        {
            os << "    Leecher's peers information: " << std::endl;
            for (Peer_t const& peer : config.peers)
                os << "    " << peer << std::endl;
        }

        return os;
    }

    void Config_t::throwException(std::exception const& e) const 
    {
        if (!helpRequested)
            throw e;
    }

    void Config_t::initFlagOptionHandlers() 
    {
        flagOptionHandler[option_help] = [&]() { helpRequested = true; };
        flagOptionHandler[option_verbose] = [&]() { enableVerbose = true; };
    }

    void Config_t::initKeyValueOptionsHandlers() 
    {
        keyValueOptionHandler[option_savefile] = [&](std::string const& value) { saveFilename = value; };
        keyValueOptionHandler[option_savefile] = [&](std::string const& value) { logFilename = value; };
        keyValueOptionHandler[option_id] = [&](std::string const& value) { clientId = std::stoi(value); };
        keyValueOptionHandler[option_bindport] = [&](std::string const& value) 
        { 
            if (!isValidPort(value))
                throwException(InvalidPortNumber(value));
            seederPort = std::stoi(value);
        };
        keyValueOptionHandler[option_peers] = [&](std::string const& value)
        {
            if (peers.size() >= Defaults::MaxConnections)
                throwException(PeerLimitExceeded(Defaults::MaxConnections));
            auto tokens = splitIntoIPAndPort(value);
            peers.push_back(Peer_t(tokens[0], std::stoi(tokens[1])));
        };
    }

    void Config_t::initOptionHandlers() 
    {
        initFlagOptionHandlers();
        initKeyValueOptionsHandlers();
    }

    void Config_t::parseOptions(std::vector<std::string> const& options,
                                bool const skipKeyValueOptions)
    {
        for (unsigned int i = 0; i < options.size(); i++)
        {
            if (options[i][0] != '-' && torrentFilename.empty())
                torrentFilename = options[i];
            else if (isFlagOption(options[i])) 
            {
                auto handler = flagOptionHandler.find(options[i]);
                if (handler != flagOptionHandler.end())
                    handler->second();
            }
            else if (isKeyValueOption(options[i]) && 
                     i + 1 < options.size() && options[i+1][0] != '-') 
            {
                i++;
                auto handler = keyValueOptionHandler.find(options[i-1]);
                if (handler != keyValueOptionHandler.end() &&
                    !skipKeyValueOptions) 
                {
                    handler->second(options[i]);
                }
            }
            else 
            {
                throwException(BadOption(options[i]));
            }
        }

        if (torrentFilename.empty())
            throwException(TorrentFileMissing());

        if (saveFilename.empty()) 
            saveFilename = torrentFilename;
    }

    bool Config_t::isFlagOption(std::string const& option) const 
    {
        for (auto itr = flagOptionHandler.cbegin(); itr != flagOptionHandler.cend(); ++itr)
            if (itr->first == option)
                return true;
        return false;
    }

    bool Config_t::isKeyValueOption(std::string const& option) const 
    {
        for (auto itr = keyValueOptionHandler.cbegin(); itr != keyValueOptionHandler.cend(); ++itr)
            if (itr->first == option)
                return true;
        return false; 
    }

    bool Config_t::isSeeder() const 
    {
        return peers.empty();
    }

    Config_t::Config_t(std::vector<std::string> const& options)
        : helpRequested(false),
          enableVerbose(false),
          clientId(0),
          seederPort(Defaults::DefaultPort),
          logFilename(Defaults::defaultLogFilename)
    {
        if (options.size() == 0) 
        {
            helpRequested = true;
            return;
        }

        initOptionHandlers();
        
        parseOptions(options, true);
        if (helpRequested) 
        {
            return;
        }

        parseOptions(options, false);
    }

    void Config_t::startSeeder(Torrent_t const& torrent) const 
    {
        Seeder_t s(torrent, seederPort);
        s.startTransfer();
    }

    void Config_t::startLeechers(Torrent_t const& torrent) const 
    {
        for (auto& seeder : peers) 
        {
            Leecher_t l(torrent, seeder);
            l.startTransfer();
        }
    }

    void Config_t::StartPeer() const 
    {
        if (helpRequested) 
        {
            std::cout << GetHelpMesssage() << std::endl;
            return;
        }

        STATUSCODE status = STATUSCODE::SC_SUCCESS;
        Torrent_t const torrent(torrentFilename, status);
        if (SC_FAILED(status))
        {
            return;
        }

        if (enableVerbose) 
        {
            std::cout << *this << std::endl;
            std::cout << torrent << std::endl;
        }

        isSeeder() ? startSeeder(torrent) : startLeechers(torrent);
    }
}

