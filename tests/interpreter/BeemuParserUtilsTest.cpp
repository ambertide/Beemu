#include "../../src/libbeemu/device/processor/interpreter/parser/parse_common.h"
#include <BeemuTest.hpp>
#include "BeemuParserUtilsTest.hpp"
#include <gtest/gtest.h>


namespace BeemuTests {

std::vector<std::tuple<BeemuRegister_16, std::tuple<BeemuRegister_8, BeemuRegister_8>>> BeemuExplodeBeemuParamParameterizedTestFixture::register_test_cases = {
	{BEEMU_REGISTER_BC, {BEEMU_REGISTER_B, BEEMU_REGISTER_C}},
	{BEEMU_REGISTER_DE, {BEEMU_REGISTER_D, BEEMU_REGISTER_E}},
	{BEEMU_REGISTER_HL, {BEEMU_REGISTER_H, BEEMU_REGISTER_L}}
};

std::vector<std::tuple<uint16_t, std::tuple<uint8_t, uint8_t>>> BeemuExplodeBeemuParamUint16ParameterizedTestFixture::uint_test_cases = {
	{0xABCD, {0xAB, 0xCD}},
	{0x00AB, {0x00, 0xAB}},
	{0xAB00, {0xAB, 0x00}},
	{0x0000, {0x00, 0x00}}
};

TEST_P(BeemuExplodeBeemuParamParameterizedTestFixture, ShouldExplodeSixteenBitRegistersToTwoEightBitRegisters)
{
	auto [compound_register, base_registers] = GetParam();
	auto [higher_register, lower_register] = base_registers;
	BeemuParam param;
	param.type = BEEMU_PARAM_TYPE_REGISTER_16;
	param.pointer = false;
	param.value.register_16 = compound_register;
	const auto [higher, lower] = beemu_explode_beemu_param(&param, &processor.processor());
	EXPECT_EQ(higher.type, BEEMU_PARAM_TYPE_REGISTER_8);
	EXPECT_EQ(lower.type, BEEMU_PARAM_TYPE_REGISTER_8);
	EXPECT_EQ(higher.pointer, false);
	EXPECT_EQ(higher.value.register_8, higher_register);
	EXPECT_EQ(lower.value.register_8, lower_register);
}

TEST_P(BeemuExplodeBeemuParamUint16ParameterizedTestFixture, ShouldExplodeSixteenBitNumberToTwoEightBitNumbers)
{
	auto [sword, bytes] = GetParam();
	auto [msb, lsb] = bytes;
	BeemuParam param;
	param.type = BEEMU_PARAM_TYPE_UINT16;
	param.pointer = false;
	param.value.value = sword;
	const auto [higher, lower] = beemu_explode_beemu_param(&param, &processor.processor());
	EXPECT_EQ(higher.type, BEEMU_PARAM_TYPE_UINT_8);
	EXPECT_EQ(lower.type, BEEMU_PARAM_TYPE_UINT_8);
	EXPECT_EQ(higher.pointer, false);
	EXPECT_EQ(higher.value.value, msb);
	EXPECT_EQ(lower.value.value, lsb);
}

TEST_F(BeemuExplodeBeemuParamTestFixture, ShouldExplodeSPRegisterToTwoUint8s)
{
	constexpr BeemuParam param { false, BEEMU_PARAM_TYPE_REGISTER_16, { .register_16 = BEEMU_REGISTER_SP } };
	const auto processor_state = processor.processor();
	const auto [higher, lower] = beemu_explode_beemu_param(&param, &processor_state);
	EXPECT_EQ(higher.type, BEEMU_PARAM_TYPE_UINT_8);
	EXPECT_EQ(lower.type, BEEMU_PARAM_TYPE_UINT_8);
	EXPECT_EQ(higher.pointer, false);
	EXPECT_EQ(lower.pointer, false);
	EXPECT_EQ(higher.value.value, (processor_state.registers->stack_pointer & 0xFF00) >> 8);
	EXPECT_EQ(lower.value.value, (processor_state.registers->stack_pointer & 0x00FF));

}

INSTANTIATE_TEST_SUITE_P(
	BeemuExplodeBeemuParamRegisterTests,
	BeemuExplodeBeemuParamParameterizedTestFixture,
	::testing::ValuesIn(
		BeemuExplodeBeemuParamParameterizedTestFixture::register_test_cases.begin(),
		BeemuExplodeBeemuParamParameterizedTestFixture::register_test_cases.end())
	);

INSTANTIATE_TEST_SUITE_P(
	BeemuExplodeBeemuParamUintTests,
	BeemuExplodeBeemuParamUint16ParameterizedTestFixture,
	::testing::ValuesIn(
		BeemuExplodeBeemuParamUint16ParameterizedTestFixture::uint_test_cases.begin(),
		BeemuExplodeBeemuParamUint16ParameterizedTestFixture::uint_test_cases.end())
	);
}
