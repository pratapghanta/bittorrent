#include <iostream>
#include <fstream>

#include "gtest/gtest.h"
#include "torrent/Metainfo.hpp"
#include "torrent/MinimalTorrentParser.hpp"

namespace BT { namespace Test {
    using ValueParameter_t = std::map<std::string, std::string>::value_type;

    const std::string testFileDir = "/tmp/";
    const std::string fileSeparator = "/";

    class TestMinimalTorrentParser
        : // public ::testing::Test,
          public ::testing::TestWithParam<ValueParameter_t> {
    protected:
        std::ofstream mofs;
    };

    TEST_P(TestMinimalTorrentParser, Integer) {
        ValueParameter_t param = GetParam();
        
        std::string strFileName = testFileDir + fileSeparator + param.first;
        mofs.open(strFileName);
        mofs << "d3:keyi" << param.second << "ee";
        mofs.close();

        MinimalTorrentParser_t parser;
        Metainfo_t info;

        parser.Parse(strFileName, info);
        EXPECT_EQ((info.mData["key"]).GetInt(), std::stol(param.second));
    }

    std::map<std::string, std::string> positiveValue {
        { "negative", "-1" },
        { "zero", "0" },
        { "positive", "1" },
        { "signed", "+1" },
        { "decimal", "1.23" }
    };

    std::map<std::string, std::string> negativeValues {
        { "empty", "" }
    };

    INSTANTIATE_TEST_SUITE_P(positiveValues,
                             TestMinimalTorrentParser,
                             testing::ValuesIn(positiveValue));
}}