#include "BeemuParserTest.hpp"
#include "../../src/libbeemu/device/processor/interpreter/parser.h"

namespace BeemuTests
{
TEST_P(BeemuParserParameterizedTestFixture, TokenParsedCorrectly)
{
	auto params = GetParam();
	auto instruction = std::get<0>(params);
	auto processor = std::get<1>(params);
	auto expected_commands = std::get<2>(params);
	// Get the actual parsed commands.
	auto actual_commands = beemu_parser_parse(&processor, &instruction);
	// Check if, you know, they match at all.
	while (!beemu_command_queue_is_empty(actual_commands)) {
		auto expected_command = beemu_command_queue_dequeue(&expected_commands);
		auto actual_command = beemu_command_queue_dequeue(actual_commands);
		ASSERT_EQ(*expected_command, *actual_command);
		free(expected_command);
		free(actual_command);
	}

	if (!beemu_command_queue_is_empty(&expected_commands)) {
		ASSERT_FALSE(true);
	}
	beemu_command_queue_free(actual_commands);

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