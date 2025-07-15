#ifndef BEEMU_BEEMU_PARSER_TEST
#define BEEMU_BEEMU_PARSER_TEST
#include "../../src/libbeemu/device/processor/interpreter/command.h"
#include "../include/BeemuCommandSerializers.hpp"
#include "../utilities/BeemuProcessorPreset.hpp"

#include <gtest/gtest.h>
#include <libbeemu/device/processor/processor.h>

struct InterpreterCommandTest {
	BeemuInstruction token;
	std::string processor;
	std::vector<BeemuMachineCommand> command_queue;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	InterpreterCommandTest,
	token,
	processor,
	command_queue);

struct InterpreterCommandTestFile {
	std::vector<InterpreterCommandTest> commands;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	InterpreterCommandTestFile,
	commands);

namespace BeemuTests {

	/**
	* @brief Get the commands tests stored in the file, construct necessary command queues and fetch
	* processor presets and run the tests.
	*
	* @return std::tuple<BeemuInstruction, BeemuProcessor, BeemuCommandQueue>>
	*/
	std::vector<std::tuple<BeemuInstruction, BeemuProcessor, BeemuCommandQueue>> getCommandsFromTestFile()
	{
		std::string test_file_path = PATH_TO_TEST_RESOURCES;
		test_file_path += "/command_tests.json";
		std::ifstream test_file(test_file_path);
		auto parsed_test_data = nlohmann::json::parse(test_file);
		InterpreterCommandTestFile test_data;
		::from_json(parsed_test_data, test_data);
		std::vector<std::tuple<BeemuInstruction, BeemuProcessor, BeemuCommandQueue>> new_vector;
		for (const InterpreterCommandTest &test_case : test_data.commands)
		{
			auto processor = new BeemuProcessorPreset(test_case.processor);
			auto queue = beemu_command_queue_new();
		    for (auto command : test_case.command_queue) {
		    	beemu_command_queue_enqueue(queue, &command);
		    }
			auto tuple = std::make_tuple(test_case.token, processor->processor(), *queue);
			new_vector.push_back(tuple);
		}
		return new_vector;
	}

	class BeemuParserParameterizedTestFixture : public ::testing::TestWithParam<std::tuple<BeemuInstruction, BeemuProcessor, BeemuCommandQueue>>
	{
	protected:
		void SetUp() override {}
		void TearDown() override {}
	};
}

#endif // BEEMU_BEEMU_PARSER_TEST