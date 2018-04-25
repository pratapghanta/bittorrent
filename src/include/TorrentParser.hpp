#ifndef TORRENT_PARSER_HPP
#define TORRENT_PARSER_HPP

#include "Metainfo.hpp"

namespace BT {
	/* Interface to represent algorithms for parsing .torrent file   */
	/*                                                               */
	/* Implementation for this project is done only to support       */
	/* parsing of standard fields in .torrent file.                  */
	/*                                                               */
	/* Room for improving the algorithm is created by programming to */
	/* interface so that algorithm can be changed with less ripples. */

	class TorrentParser {
	public:
		virtual Metainfo const doParse(std::string const& fileName) const = 0;
	};
}
#endif
