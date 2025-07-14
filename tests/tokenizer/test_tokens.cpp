#include "libbeemu/device/processor/processor.h"

#include <BeemuTokenTest.hpp>
#include <libbeemu/device/processor/tokenizer.h>

namespace BeemuTests
{
	TEST_P(BeemuTokenParameterizedTestFixture, InstructionTokenizedCorrectly)
	{
		auto params = GetParam();
		auto machine_code = params.first;
		auto expected_instruction = params.second;
		auto inst = beemu_tokenizer_tokenize(machine_code);
		ASSERT_EQ(*inst, expected_instruction);
		beemu_tokenizer_free_token(inst);
	}

	auto tests = BeemuTests::getTokensFromTestFile();

	INSTANTIATE_TEST_SUITE_P(
		BeemuTokenizerTests,
		BeemuTokenParameterizedTestFixture,
		::testing::ValuesIn(tests.begin(), tests.end()),
		[](const testing::TestParamInfo<BeemuTokenParameterizedTestFixture::ParamType> &info)
		{
			// Can use info.param here to generate the test suffix
			std::stringstream stream;
			stream << "0x" << std::hex << std::uppercase << info.param.second.original_machine_code;
			std::string name = stream.str();
			return name;
		});
}