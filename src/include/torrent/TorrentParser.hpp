#if !defined(TORRENT_PARSER_HPP)
#define TORRENT_PARSER_HPP

#include <string>

#include "common/StatusCode.hpp"

namespace BT 
{
    class Metainfo;
    class ITorrentParser 
    {
    public:
        virtual STATUSCODE Parse(std::string const&, Metainfo&) const = 0;
    };
}

#endif // !defined(TORRENT_PARSER_HPP)
