#if !defined(TORRENT_PARSER_HPP)
#define TORRENT_PARSER_HPP

#include "common/StatusCode.hpp"

namespace BT 
{
    class Metainfo;

    class ITorrentParser 
    {
    public:
        virtual STATUSCODE Parse(/* IN */  std::string const& fileName,
                                 /* OUT */ Metainfo& rInfo) const = 0;
    };
}

#endif // !defined(TORRENT_PARSER_HPP)
