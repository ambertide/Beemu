#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/instruction.h>
#include <BeemuTest.hpp>
#include <stdbool.h>
#include <gtest/gtest.h>

namespace BeemuTests
{
	BeemuInstruction generate_jump_instruction(BeemuParam addr, BeemuJumpType jumpType = BEEMU_JUMP_TYPE_JUMP, bool is_relative = false, BeemuJumpCondition condition = BEEMU_JUMP_IF_NO_CONDITION)
	{
		BeemuInstruction instruction;
		instruction.type = BEEMU_INSTRUCTION_TYPE_JUMP;
		instruction.params.jump_params = {
			.param = addr,
			.type = jumpType,
			.condition = condition,
			.is_conditional = condition != BEEMU_JUMP_IF_NO_CONDITION,
			.is_relative = false,
			.enable_interrupts = false};
		return instruction;
	}

	BeemuParam generate_jump_param(uint16_t addr)
	{
		BeemuParam jump_location;
		jump_location.type = BEEMU_PARAM_TYPE_UINT16;
		jump_location.value.value = addr;
		return jump_location;
	}

	TEST_F(BeemuTestFixture, JumpDirectTest)
	{
		beemu_registers_write_register_value(registers, pc, 0xAA);
		BeemuParam jump_location = generate_jump_param(0xFF);
		execute_instruction(memory, registers, generate_jump_instruction(jump_location));
		// Check if we, you know, jumped at all.
		EXPECT_EQ(beemu_registers_read_register_value(registers, pc), 0xFF);
	}

	TEST_F(BeemuTestFixture, JumpDirectSPEffectTest)
	{
		// Lets make sure that the stack pointer does not move
		// on direct jump.
		const uint8_t previous_sp = beemu_registers_read_register_value(registers, sp);
		const uint16_t previous_sp_val = beemu_memory_read_16(memory, previous_sp);
		BeemuParam jump_location = generate_jump_param(0xFF);
		execute_instruction(memory, registers, generate_jump_instruction(jump_location));
		const uint8_t current_sp = beemu_registers_read_register_value(registers, sp);
		const uint16_t current_sp_val = beemu_memory_read_16(memory, current_sp);
		EXPECT_EQ(previous_sp, current_sp);
		EXPECT_EQ(previous_sp_val, current_sp_val);
	}

	/**
	 *  Check if direct calls result in return address properly pushed to the
	 * 	stack.
	 */
	TEST_F(BeemuTestFixture, CallDirectTest)
	{
		BeemuInstruction jump_instruction = generate_jump_instruction(
			generate_jump_param(0xAA),
			BEEMU_JUMP_TYPE_CALL);
		const uint16_t previous_pc = beemu_registers_read_register_value(registers, pc);
		const uint16_t previous_sp = beemu_registers_read_register_value(registers, sp);
		execute_instruction(memory, registers, jump_instruction);
		const uint16_t current_sp = beemu_registers_read_register_value(registers, sp);
		const uint16_t val_at_previous_sp = beemu_memory_read_16(memory, previous_sp);
		// Expect stack to have one more value (space for RET value.)
		EXPECT_EQ(current_sp, previous_sp - 2);
		// Expect the last filled stack value (which happens to be the
		// memory addr at the prev sp value) to hold the memory location
		// of the instruction immediatelly following the CALL instruction.
		EXPECT_EQ(previous_pc + 3, val_at_previous_sp);
		// Expect a jump to have occurred.
		EXPECT_EQ(beemu_registers_read_register_value(registers, pc), 0xAA);
	}

	TEST_F(BeemuTestFixture, Ret)
	{
		BeemuInstruction call_instruction = generate_jump_instruction(
			generate_jump_param(0xAA),
			BEEMU_JUMP_TYPE_CALL);
		const uint16_t previous_pc = beemu_registers_read_register_value(registers, pc);
		const uint16_t previous_sp = beemu_registers_read_register_value(registers, sp);
		// First note the existing program counter and jump to a location.
		execute_instruction(memory, registers, call_instruction);
		// Second construct the RET instruction and check if program counter
		// resetted properly.
		// the below 3 lines do nothing.
		BeemuInstruction ret_instruction = generate_jump_instruction(
			generate_jump_param(0x00),
			BEEMU_JUMP_TYPE_RET);
		execute_instruction(memory, registers, ret_instruction);
		const uint16_t current_pc = beemu_registers_read_register_value(registers, pc);
		EXPECT_EQ(previous_pc + 3, current_pc);
		// Third, check if the stack was popped, ie, returned to its previous position.
		const uint16_t current_sp = beemu_registers_read_register_value(registers, sp);
		EXPECT_EQ(current_sp, previous_sp);
	}
}