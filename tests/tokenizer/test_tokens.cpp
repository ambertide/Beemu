#include <BeemuTokenTest.hpp>
#include <libbeemu/device/processor/tokenizer.h>

namespace BeemuTests
{
	TEST_P(BeemuTokenParameterizedTestFixture, InstructionTokenizedCorrectly)
	{
		auto params = GetParam();
		auto machine_code = params.first;
		auto expected_instruction = params.second;
		ASSERT_EQ(beemu_tokenizer_tokenize_new(machine_code), expected_instruction);
	}

	INSTANTIATE_TEST_SUITE_P(
		BeemuTokenizerTests,
		BeemuTokenParameterizedTestFixture,
		::testing::ValuesIn(BeemuTests::getTokensFromTestFile()));

}