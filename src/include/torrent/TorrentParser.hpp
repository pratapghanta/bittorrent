#if !defined(TORRENT_PARSER_HPP)
#define TORRENT_PARSER_HPP

#include "common/StatusCode.hpp"

namespace BT 
{
    class Metainfo_t;

    /* Implementation for this project is done only to support       */
    /* parsing of basic fields in .torrent file.                     */
    class ITorrentParser 
    {
    public:
        virtual STATUSCODE Parse(/* IN */  std::string const& fileName,
                                 /* OUT */ Metainfo_t& rInfo) const = 0;
    };
}
#endif // !defined(TORRENT_PARSER_HPP)
