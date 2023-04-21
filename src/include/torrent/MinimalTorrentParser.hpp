#if !defined(MINIMAL_TORRENT_PARSER_HPP)
#define MINIMAL_TORRENT_PARSER_HPP

#include <iostream>
#include <fstream>

#include "torrent/Metainfo.hpp"
#include "torrent/TorrentParser.hpp"

namespace BT 
{
    class MinimalTorrentParser : public ITorrentParser 
    {
    public:
        MinimalTorrentParser();
        ~MinimalTorrentParser() = default;

        MinimalTorrentParser(const MinimalTorrentParser&) = delete;
        MinimalTorrentParser& operator=(const MinimalTorrentParser&) = delete;

        MinimalTorrentParser(MinimalTorrentParser&&);
        MinimalTorrentParser& operator=(MinimalTorrentParser&&);

        STATUSCODE Parse(std::string const& fileName,
                         Metainfo& rInfo) const override;

    private:
        void reset();
    
        STATUSCODE openFile(std::string const& strFileName) const;
        STATUSCODE computeInfoDictHash(std::string& rHash) const;

        STATUSCODE extractChar(char& ch) const;
        STATUSCODE extractLong(char const delim,
                               long& rOut) const;

        MI_Object extract_MI_Int() const;
        MI_Object extract_MI_String() const;
        MI_Object extract_MI_List() const;
        MI_Object extract_MI_Dict() const;

        MI_Object extractData(char ch) const;
        
        mutable std::ifstream mIfstream;
        mutable bool mInfoDict;
        mutable std::streampos mStartInfoDict;
        mutable std::streampos mEndInfoDict;
    };
}

#endif // !defined(MINIMAL_TORRENT_PARSER_HPP)
