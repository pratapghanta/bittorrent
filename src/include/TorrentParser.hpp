#if !defined(TORRENT_PARSER_HPP)
#define TORRENT_PARSER_HPP

#include "Errors.hpp"
#include "Metainfo.hpp"

namespace BT {
    /* Interface to represent algorithms for parsing .torrent file   */
    /*                                                               */
    /* Implementation for this project is done only to support       */
    /* parsing of basic fields in .torrent file.                     */
    /*                                                               */
    /* Room for improving the algorithm is created by programming to */
    /* interface so that algorithm can be changed with less ripples. */

    class TorrentParser_t {
    public:
        virtual STATUSCODE Parse(/* IN */  std::string const& fileName,
                                 /* OUT */ Metainfo_t& rInfo) const = 0;
    };
}
#endif // !defined(TORRENT_PARSER_HPP)
