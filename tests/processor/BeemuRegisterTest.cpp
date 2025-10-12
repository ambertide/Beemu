#include "../include/BeemuTest.hpp"
#include <gtest/gtest.h>
#include <beemu/device/primitives/register.h>

namespace BeemuTests
{
	TEST_F(BeemuTestFixture, RegistersWriteReadTest8)
	{
		beemu_registers_write_register_value(registers, a, 8);
		const uint16_t value = beemu_registers_read_register_value(registers, a);
		EXPECT_EQ(value, 8);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_A], 8);
	}

	TEST_F(BeemuTestFixture, RegistersWriteTest16)
	{
		beemu_registers_write_register_value(registers, hl, 0xAAFF);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_H], 0xAA);
		EXPECT_EQ(registers->registers[BEEMU_REGISTER_L], 0xFF);
	}

	TEST_F(BeemuTestFixture, RegisterReadTest16)
	{
		registers->registers[BEEMU_REGISTER_H] = 0xAA;
		registers->registers[BEEMU_REGISTER_L] = 0xFF;
		EXPECT_EQ(beemu_registers_read_register_value(registers, hl), 0xAAFF);
	}

	TEST_F(BeemuTestFixture, RegisterReadWriteTest16)
	{
		beemu_registers_write_register_value(registers, hl, 0xAAFF);
		EXPECT_EQ(beemu_registers_read_register_value(registers, hl), 0xAAFF);
	}

	TEST_F(BeemuTestFixture, FlagSetTest)
	{
		beemu_registers_flags_set_flag(registers, BEEMU_FLAG_Z, 1);
		EXPECT_EQ(registers->flags, 0b10000000);
	}

	TEST_F(BeemuTestFixture, FlagGetTest)
	{
		registers->flags = 0b10000000;
		uint8_t z_flag = beemu_registers_flags_get_flag(registers, BEEMU_FLAG_Z);
		EXPECT_EQ(z_flag, 1);
	}

	TEST_F(BeemuTestFixture, FlagGetSetTest)
	{
		beemu_registers_flags_set_flag(registers, BEEMU_FLAG_Z, 1);
		uint8_t z_flag = beemu_registers_flags_get_flag(registers, BEEMU_FLAG_Z);
		EXPECT_EQ(z_flag, 1);
	}
}
