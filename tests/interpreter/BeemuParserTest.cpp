#include "BeemuParserTest.hpp"

namespace BeemuTests
{
TEST_P(BeemuParserParameterizedTestFixture, TokenParsedCorrectly)
{
	auto params = GetParam();
	auto instruction = std::get<0>(params);
	auto processor = std::get<1>(params);
	auto commands = std::get<2>(params);
	ASSERT_FALSE(true);
}

auto parser_tests = BeemuTests::getCommandsFromTestFile();

INSTANTIATE_TEST_SUITE_P(
	BeemuParserTests,
	BeemuParserParameterizedTestFixture,
	::testing::ValuesIn(parser_tests.begin(), parser_tests.end()),
	[](const testing::TestParamInfo<BeemuParserParameterizedTestFixture::ParamType> &info)
	{
		// Can use info.param here to generate the test suffix
		std::stringstream stream;
		stream << "0x" << std::hex << std::uppercase << std::get<0>(info.param).original_machine_code;
		std::string name = stream.str();
		return name;
	});
}