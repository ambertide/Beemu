#include <libbeemu/device/processor/registers.h>
#include <libbeemu/device/memory.h>
#include <gtest/gtest.h>

namespace BeemuTests
{
	class BeemuTestFixture : public ::testing::Test
	{
	protected:
		BeemuMemory *memory;
		BeemuRegisters *registers;
		BeemuRegister hl = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_HL};
		BeemuRegister a = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_A};
		BeemuRegister b = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_B};
		BeemuRegister c = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_C};
		BeemuRegister d = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_D};
		BeemuRegister h = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_H};
		BeemuRegister l = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_L};
		void SetUp() override;

		void TearDown() override;
	};
};
