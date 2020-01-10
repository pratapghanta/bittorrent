#ifndef TORRENT_PARSER_HPP
#define TORRENT_PARSER_HPP

#include "StatusCode.hpp"
#include "Metainfo.hpp"

namespace BT {
	/* Interface to represent algorithms for parsing .torrent file   */
	/*                                                               */
	/* Implementation for this project is done only to support       */
	/* parsing of standard fields in .torrent file.                  */
	/*                                                               */
	/* Room for improving the algorithm is created by programming to */
	/* interface so that algorithm can be changed with less ripples. */

	class TorrentParser_t {
	public:
		virtual STATUSCODE Parse(/* IN */  std::string const& fileName,
		                         /* OUT */ Metainfo_t& rInfo) const = 0;
	};
}
#endif // #ifndef TORRENT_PARSER_HPP
