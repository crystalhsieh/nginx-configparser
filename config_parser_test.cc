#include "gtest/gtest.h"
#include "config_parser.h"

TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}

TEST(NginxConfigStringTest, ToString) {
  NginxConfigStatement statement;
  statement.tokens_.push_back("foo");
  statement.tokens_.push_back("bar");
  EXPECT_EQ(statement.ToString(0), "foo bar;\n");
}

// Standardizes the method to parse strings from a string stream.
class NginxInputConfigTest : public ::testing::Test {
protected:
  bool ParseString(const std::string config_string) {
    std::stringstream config_stream(config_string);
    return parser_.Parse(&config_stream, &out_config_);
  }
  NginxConfigParser parser_;
  NginxConfig out_config_;
};

// Decided that if there's nothing in the config, return true.
TEST_F(NginxInputConfigTest, EmptyConfig) {
  EXPECT_TRUE(ParseString(""));
}

TEST_F(NginxInputConfigTest, ParseStringConfig) {
  EXPECT_TRUE(ParseString("foo bar;"));
}

TEST_F(NginxInputConfigTest, NoSemicolon) {
  EXPECT_FALSE(ParseString("foo bar"));
}

// Many versions of the same issue - unbalanced number of brackets
TEST_F(NginxInputConfigTest, UnbalancedBrackets) {
  EXPECT_FALSE(ParseString("listen { foo bar;"));
  EXPECT_FALSE(ParseString("listen foo bar; }"));
  EXPECT_FALSE(ParseString("listen { foo bar; }}"));
  EXPECT_FALSE(ParseString("{ listen { foo bar; }"));
}

TEST_F(NginxInputConfigTest, BalancedBrackets) {
  EXPECT_TRUE(ParseString("listen { foo bar; }"));
}

TEST_F(NginxInputConfigTest, InvalidOrderOfBrackets) {
  EXPECT_FALSE(ParseString("}foo bar;{"));
}

TEST_F(NginxInputConfigTest, NestedBrackets) {
  EXPECT_TRUE(ParseString("listen {expect { foo bar; }}"));
}

TEST_F(NginxInputConfigTest, NestedBracketsMultipleCommands) {
  EXPECT_TRUE(ParseString("listen {expect { foo bar; } from me; }"));
}

TEST_F(NginxInputConfigTest, MultipleNestedBrackets) {
  EXPECT_TRUE(ParseString("listen {expect { foo bar; } from { this place; } }"));
}

// Comment is valid
TEST_F(NginxInputConfigTest, ValidComment) {
  EXPECT_TRUE(ParseString("# This is a comment."));
}

// Comment is separated from the code.
TEST_F(NginxInputConfigTest, ValidCommentWithCode) {
  EXPECT_TRUE(ParseString("foo bar; # This is a comment."));
}
