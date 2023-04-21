#include "common/Defaults.hpp"

namespace BT
{
    void printDefaults(std::ostream& os) 
    {
        os << "*** BitTorrent application configuration ***" << std::endl;
        os << "    Default log file name: " << Defaults::DefaultLogFilename << std::endl;
        os << "    Maximum internal buffer size: " << Defaults::MaxBufferSize << std::endl;
        os << "    Maximum number of peer connections: " << Defaults::MaxConnections << std::endl;
        os << "    Port range: " << Defaults::InitPort << "-" << Defaults::MaxPort << std::endl;
        os << "    Default port: " << Defaults::DefaultPort << std::endl;
        os << "    Maximum supported size of internal buffers:" << std::endl;
        os << "        Block length: " << Defaults::BlockSize << std::endl;
        os << "        SHA1 Hash size: " << Defaults::Sha1MdSize << std::endl;
    }
}