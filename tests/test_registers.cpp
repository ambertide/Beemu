#include <gtest/gtest.h>
#include <libbeemu/device/primitives/register.h>
#include "include/BeemuTest.hpp"

namespace BeemuTests
{
	TEST_F(BeemuTestFixture, RegistersWriteReadTest8)
	{
		BeemuRegister register_ = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_A};
		beemu_registers_write_register_value(registers, register_, 8);
		const uint16_t value = beemu_registers_read_register_value(registers, register_);
		EXPECT_EQ(value, 8);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_A], 8);
	}

	TEST_F(BeemuTestFixture, RegistersWriteReadTest16)
	{
		BeemuRegister register_ = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_HL};
		beemu_registers_write_register_value(registers, register_, 0xAAFF);
		const uint16_t value = beemu_registers_read_register_value(registers, register_);
		EXPECT_EQ(value, 0xAAFF);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_H], 0xFF);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_L], 0xAA);
	}
}
