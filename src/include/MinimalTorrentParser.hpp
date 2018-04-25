#ifndef MINIMAL_TORRENT_PARSER_HPP
#define MINIMAL_TORRENT_PARSER_HPP

#include "TorrentParser.hpp"

namespace BT {
	class MinimalTorrentParser : public TorrentParser {
	public:
		Metainfo const doParse(std::string const& fileName) const override;
	};
}
#endif
