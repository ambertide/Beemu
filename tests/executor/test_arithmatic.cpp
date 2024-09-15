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
		BeemuParam register_param;
		register_param.type = BEEMU_PARAM_TYPE_REGISTER_8;
		register_param.value.register_8 = BEEMU_REGISTER_A;
		BeemuParam uint8_param;
		uint8_param.type = BEEMU_PARAM_TYPE_UINT_8;
		uint8_param.value.value = 15;
		BeemuInstruction instruction = generate_arithmatic_instruction(
			register_param,
			uint8_param,
			BEEMU_OP_ADD,
			false);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(30, beemu_registers_read_register_value(registers, a));
	}

	TEST_F(BeemuTestFixture, InstructionArithmaticAddOverflow)
	{
		beemu_registers_write_register_value(registers, a, 250);
		BeemuParam register_param;
		register_param.type = BEEMU_PARAM_TYPE_REGISTER_8;
		register_param.value.register_8 = BEEMU_REGISTER_A;
		BeemuParam uint8_param;
		uint8_param.type = BEEMU_PARAM_TYPE_UINT_8;
		uint8_param.value.value = 15;
		BeemuInstruction instruction = generate_arithmatic_instruction(
			register_param,
			uint8_param,
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
		BeemuParam register_hl_param;
		register_hl_param.type = BEEMU_PARAM_TYPE_REGISTER_16;
		register_hl_param.value.register_16 = BEEMU_REGISTER_HL;
		// Now construct the ptr parameter.
		BeemuParam ptr_param;
		ptr_param.pointer = true;
		ptr_param.type = BEEMU_PARAM_TYPE_UINT_8;
		ptr_param.value.value = 15;
		BeemuInstruction instruction = generate_arithmatic_instruction(
			register_hl_param,
			ptr_param,
			BEEMU_OP_ADD,
			true);
		execute_instruction(memory, registers, instruction);
		EXPECT_EQ(0x100F, beemu_registers_read_register_value(registers, hl));
		EXPECT_EQ(0x0, beemu_registers_flags_get_flag(registers, BEEMU_FLAG_C));
	}
}
