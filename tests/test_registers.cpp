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

	TEST_F(BeemuTestFixture, RegistersWriteTest16)
	{
		beemu_registers_write_register_value(registers, hl_register, 0xAAFF);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_H], 0xAA);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_L], 0xFF);
	}

	TEST_F(BeemuTestFixture, RegisterReadTest16)
	{
		registers->registers[BEEMU_REGISTER_H] = 0xAA;
		registers->registers[BEEMU_REGISTER_L] = 0xFF;
		EXPECT_EQ(beemu_registers_read_register_value(registers, hl_register), 0xAAFF);
	}

	TEST_F(BeemuTestFixture, RegisterReadWriteTest16)
	{
		beemu_registers_write_register_value(registers, hl_register, 0xAAFF);
		EXPECT_EQ(beemu_registers_read_register_value(registers, hl_register), 0xAAFF);
	}
}
