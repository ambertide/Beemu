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

	TEST_F(BeemuTestFixture, JumpDirectTest)
	{
		beemu_registers_write_register_value(registers, pc, 0xAA);
		execute_instruction(memory, registers, generate_jump_instruction((BeemuParam){.type = BEEMU_PARAM_TYPE_UINT16, .value = 0xFF}));
		// Check if we, you know, jumped at all.
		EXPECT_EQ(beemu_registers_read_register_value(registers, pc), 0xFF);
	}

	TEST_F(BeemuTestFixture, JumpDirectSPEffectTest)
	{
		// Lets make sure that the stack pointer does not move
		// on direct jump.
		const uint8_t previous_sp = beemu_registers_read_register_value(registers, sp);
		const uint16_t previous_sp_val = beemu_memory_read_16(memory, previous_sp);
		execute_instruction(memory, registers, generate_jump_instruction((BeemuParam){.type = BEEMU_PARAM_TYPE_UINT16, .value = 0xFF}));
		const uint8_t current_sp = beemu_registers_read_register_value(registers, sp);
		const uint16_t current_sp_val = beemu_memory_read_16(memory, current_sp);
		EXPECT_EQ(previous_sp, current_sp);
		EXPECT_EQ(previous_sp_val, current_sp_val);
	}
}