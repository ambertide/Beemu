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
	}

	auto tests = BeemuTests::getTokensFromTestFile();

	INSTANTIATE_TEST_SUITE_P(
		BeemuTokenizerTests,
		BeemuTokenParameterizedTestFixture,
		::testing::ValuesIn(tests.begin(), tests.end()));
}