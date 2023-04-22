#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/instruction.h>
#include <BeemuTest.hpp>
#include <stdbool.h>
#include <gtest/gtest.h>

namespace BeemuTests
{

	BeemuInstruction generate_arithmatic_instruction(BeemuParam dest, BeemuParam source, BeemuOperation operation, bool is_16)
	{
		BeemuInstruction instruction;
		instruction.type = is_16 ? BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16 : BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8;
		instruction.params.arithmatic_params.dest = dest;
		instruction.params.arithmatic_params.source = source;
		instruction.params.arithmatic_params.operation = operation;
		return instruction;
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
