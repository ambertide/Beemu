#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/instruction.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <BeemuTest.hpp>
#include <stdbool.h>
#include <gtest/gtest.h>

namespace BeemuTests
{

	BeemuInstruction generate_load_instruction(BeemuParam dest, BeemuParam source, bool is_16)
	{
		BeemuInstruction instruction;
		instruction.type = is_16 ? BEEMU_INSTRUCTION_TYPE_LOAD_16 : BEEMU_INSTRUCTION_TYPE_LOAD_8;
		instruction.params.load_params.dest = dest;
		instruction.params.load_params.source = source;
		return instruction;
	}

	BeemuInstruction generate_arithmatic_instruction(BeemuParam dest, BeemuParam source, BeemuOperation operation, bool is_16)
	{
		BeemuInstruction instruction;
		instruction.type = is_16 ? BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16 : BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8;
		instruction.params.arithmatic_params.dest = dest;
		instruction.params.arithmatic_params.source = source;
		instruction.params.arithmatic_params.operation = operation;
		return instruction;
	}
	/**
	 * @brief Test case that covers loading immediate values to to integers.
	 */
	TEST_F(BeemuTestFixture, InstructionLoadImmediate)
	{
		BeemuInstruction instruction = {.type = BEEMU_INSTRUCTION_TYPE_LOAD_8,
										.params = {.load_params = {
													   .dest = {.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
													   .source = {.pointer = false, .type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 10}}}};
		EXPECT_EQ(0, registers->registers[BEEMU_REGISTER_A]);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(10, registers->registers[BEEMU_REGISTER_A]);
	}

	/**
	 * @brief Test case that covers loading from a register to another.
	 */
	TEST_F(BeemuTestFixture, InstructionLoadCopy)
	{
		BeemuInstruction instruction = generate_load_instruction(
			(BeemuParam){.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_B},
			(BeemuParam){.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
			false);
		EXPECT_EQ(0, registers->registers[BEEMU_REGISTER_A]);
		EXPECT_EQ(0, registers->registers[BEEMU_REGISTER_B]);
		beemu_registers_write_register_value(registers, (BeemuRegister){.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_A}, 5);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(5, registers->registers[BEEMU_REGISTER_B]);
	}

	/**
	 * @brief Test pointers for both destination and source.
	 */
	TEST_F(BeemuTestFixture, InstructionLoadWithPointers)
	{
		registers->registers[BEEMU_REGISTER_C] = 5;
		beemu_memory_write(memory, 5, 10);
		// This instruction means, load to the memory address 10,
		// the value hold in the memory address hold in the register
		// C, which is: (C) = (5) = 10.
		BeemuInstruction instruction = generate_load_instruction(
			(BeemuParam){.pointer = true, .type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 10},
			(BeemuParam){.pointer = true, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_C},
			false);
		EXPECT_EQ(0, beemu_memory_read(memory, 10));
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(10, beemu_memory_read(memory, 10));
	}

	/**
	 * @brief Test loads from and to 16 bit registers.
	 *
	 * These need to be tested seperately because they work weird.
	 */
	TEST_F(BeemuTestFixture, InstructionLoad16)
	{
		BeemuInstruction instruction = generate_load_instruction(
			(BeemuParam){.type = BEEMU_PARAM_TYPE_REGISTER_16, .value.register_16 = BEEMU_REGISTER_HL},
			(BeemuParam){.type = BEEMU_PARAM_TYPE_UINT16, .value.value = 0xAAFF},
			true);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(0xAAFF, beemu_registers_read_register_value(registers, hl));
		EXPECT_EQ(0xAA, beemu_registers_read_register_value(registers, h));
		EXPECT_EQ(0xFF, beemu_registers_read_register_value(registers, l));
	}

	TEST_F(BeemuTestFixture, InstructionArithmaticAdd)
	{
		beemu_registers_write_register_value(registers, a, 15);
		BeemuInstruction instruction = generate_arithmatic_instruction(
			(BeemuParam){.type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
			(BeemuParam){.type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 15},
			BEEMU_OP_ADD,
			false);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(30, beemu_registers_read_register_value(registers, a));
	}

	TEST_F(BeemuTestFixture, InstructionArithmaticAddOverflow)
	{
		beemu_registers_write_register_value(registers, a, 250);
		BeemuInstruction instruction = generate_arithmatic_instruction(
			(BeemuParam){.type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
			(BeemuParam){.type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 15},
			BEEMU_OP_ADD,
			false);
		// Here, we expect the value to overflow to 9
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(9, beemu_registers_read_register_value(registers, a));
		// But also, we expect the carry flag to be set to 1.
		EXPECT_EQ(1, beemu_registers_flags_get_flag(registers, BEEMU_FLAG_C));
	}

	TEST_F(BeemuTestFixture, InstructionArithmatic16)
	{
		beemu_registers_write_register_value(registers, hl, 0x1000);
		beemu_memory_write(memory, 15, 0xF);
		BeemuInstruction instruction = generate_arithmatic_instruction(
			(BeemuParam){.type = BEEMU_PARAM_TYPE_REGISTER_16, .value.register_16 = BEEMU_REGISTER_HL},
			(BeemuParam){.pointer = true, .type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 15},
			BEEMU_OP_ADD,
			true);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(0x100F, beemu_registers_read_register_value(registers, hl));
		EXPECT_EQ(0x0, beemu_registers_flags_get_flag(registers, BEEMU_FLAG_C));
	}
}
