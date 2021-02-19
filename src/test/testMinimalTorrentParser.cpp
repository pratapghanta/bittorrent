#include <iostream>
#include <fstream>

#include "gtest/gtest.h"
#include "MinimalTorrentParser.hpp"
#include "Metainfo.hpp"

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
        EXPECT_EQ(info.mData["key"].value.nInt, std::stol(param.second));
    }

    std::map<std::string, std::string> positiveValue {
        // { "empty", "" },
        // { "negative", "-1" },
        // { "zero", "0" },
        { "positive", "1" },
        // { "signed", "+1" },
        // { "decimal", "1.23" }
    };
    INSTANTIATE_TEST_SUITE_P(positiveValues,
                             TestMinimalTorrentParser,
                             testing::ValuesIn(positiveValue));
}}