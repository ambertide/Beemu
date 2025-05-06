#include <BeemuTest.hpp>
#include <gtest/gtest.h>
#include <libbeemu/device/primitives/instruction.h>
#include <libbeemu/device/processor/executor.h>
#include <stdbool.h>

namespace BeemuTests {
BeemuInstruction generate_load_instruction(BeemuParam dest, BeemuParam source, bool is_16 = false)
{
	BeemuInstruction instruction;
	instruction.type = BEEMU_INSTRUCTION_TYPE_LOAD;
	instruction.params.load_params.dest = dest;
	instruction.params.load_params.source = source;
	return instruction;
}

BeemuParam get_register_8(BeemuRegister_8 register_name, bool ptr = false)
{
	BeemuParam reg;
	reg.pointer = ptr;
	reg.type = BEEMU_PARAM_TYPE_REGISTER_8;
	reg.value.register_8 = register_name;
	return reg;
}

BeemuParam get_register_16(BeemuRegister_16 register_name, bool ptr = false)
{
	BeemuParam reg;
	reg.pointer = ptr;
	reg.type = BEEMU_PARAM_TYPE_REGISTER_16;
	reg.value.register_16 = register_name;
	return reg;
}

BeemuParam get_uint_8(uint8_t value, bool ptr = false)
{
	BeemuParam param;
	param.pointer = ptr;
	param.type = BEEMU_PARAM_TYPE_UINT_8;
	param.value.value = value;
	return param;
}

BeemuParam get_uint_16(uint16_t value, bool ptr = false)
{
	BeemuParam param;
	param.pointer = ptr;
	param.type = BEEMU_PARAM_TYPE_UINT16;
	param.value.value = value;
	return param;
}

/**
 * @brief Test case that covers loading immediate values to to integers.
 */
TEST_F(BeemuTestFixture, InstructionLoadImmediate)
{
	BeemuParam dst = get_register_8(BEEMU_REGISTER_A);
	BeemuParam src = get_uint_8(10);
	BeemuInstruction instruction = generate_load_instruction(dst, src);
	EXPECT_EQ(0x1, registers->registers[BEEMU_REGISTER_A]);
	execute_instruction(memory, registers, instruction);
	EXPECT_EQ(10, registers->registers[BEEMU_REGISTER_A]);
}

/**
 * @brief Test case that covers loading from a register to another.
 */
TEST_F(BeemuTestFixture, InstructionLoadCopy)
{
	BeemuParam dst = get_register_8(BEEMU_REGISTER_B);
	BeemuParam src = get_register_8(BEEMU_REGISTER_A);
	BeemuInstruction instruction = generate_load_instruction(
	    dst,
	    src);
	EXPECT_EQ(0x1, registers->registers[BEEMU_REGISTER_A]);
	EXPECT_EQ(0xff, registers->registers[BEEMU_REGISTER_B]);
	beemu_registers_write_register_value(registers, a, 5);
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
	BeemuParam dst = get_uint_8(10, true);
	BeemuParam src = get_register_8(BEEMU_REGISTER_C, true);
	BeemuInstruction instruction = generate_load_instruction(
	    dst,
	    src);
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
	auto dst = get_register_16(BEEMU_REGISTER_HL);
	auto src = get_uint_16(0xAAFF);
	auto instruction = generate_load_instruction(dst, src, true);
	execute_instruction(memory, registers, instruction);
	EXPECT_EQ(0xAAFF, beemu_registers_read_register_value(registers, hl));
	EXPECT_EQ(0xAA, beemu_registers_read_register_value(registers, h));
	EXPECT_EQ(0xFF, beemu_registers_read_register_value(registers, l));
}
}