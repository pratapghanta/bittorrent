#ifndef MINIMAL_TORRENT_PARSER_HPP
#define MINIMAL_TORRENT_PARSER_HPP

#include <iostream>
#include <fstream>
#include "TorrentParser.hpp"

namespace BT {
    class MinimalTorrentParser_t : public TorrentParser_t {
    public:
        MinimalTorrentParser_t();
        ~MinimalTorrentParser_t() = default;

        MinimalTorrentParser_t(/* IN */ const MinimalTorrentParser_t&) = delete;
        MinimalTorrentParser_t& operator=(/* IN */ const MinimalTorrentParser_t&) = delete;

        MinimalTorrentParser_t(/* IN */ MinimalTorrentParser_t&&);
        MinimalTorrentParser_t& operator=(/* IN */ MinimalTorrentParser_t&&);

        STATUSCODE Parse(/* IN  */  std::string const& fileName,
                         /* OUT */ Metainfo_t& rInfo) const override;

    private:
        void reset();
    
        STATUSCODE openFile(/* IN */ std::string const& strFileName) const;
        STATUSCODE computeInfoDictHash(/* OUT */ std::string& rHash) const;

        STATUSCODE extractChar(/* OUT */ char& ch) const;
        STATUSCODE extractLong(/* IN  */  char const delim,
                               /* OUT */ long& rOut) const;

        MI_Object_t extract_MI_Int() const;
        MI_Object_t extract_MI_String() const;
        MI_Object_t extract_MI_List() const;
        MI_Object_t extract_MI_Dict() const;

        MI_Object_t extractData(/* IN */ char ch) const;
        
        mutable std::ifstream mIfstream;
        mutable bool mInfoDict;
        mutable std::streampos mStartInfoDict;
        mutable std::streampos mEndInfoDict;
    };
}

#endif // #ifndef MINIMAL_TORRENT_PARSER_HPP
