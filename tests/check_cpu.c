#include <unity.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/instruction.h>
#include <stdbool.h>

BeemuMemory *memory;
BeemuRegisters *registers;

BeemuInstruction generate_load_instruction(BeemuParam dest, BeemuParam source, bool is_16)
{
	BeemuInstruction instruction;
	instruction.type = is_16 ? BEEMU_INSTRUCTION_TYPE_LOAD_16 : BEEMU_INSTRUCTION_TYPE_LOAD_8;
	instruction.params.load_params.dest = dest;
	instruction.params.load_params.source = source;
	return instruction;
}

/**
 * @brief Asser equal and print a formatted message.
 *
 */
#define TEST_ASSERT_EQUAL_MESSAGEF(expected, actual, format, ...) \
	{                                                             \
		char msg[100];                                            \
		snprintf(msg, 100, format, __VA_ARGS__);                  \
		TEST_ASSERT_EQUAL_MESSAGE(expected, actual, msg);         \
	}

/**
 * @brief Asser equal and print a formatted message.
 *
 */
#define TEST_MESSAGEF(format, ...)               \
	{                                            \
		char msg[100];                           \
		snprintf(msg, 100, format, __VA_ARGS__); \
		printf(msg);                             \
	}

void setUp(void)
{
	memory = beemu_memory_new(64000);
	registers = beemu_registers_new();
}

void tearDown(void)
{
}

/**
 * @brief Test case that covers loading immediate values to to integers.
 */
void test_instruction_load_immediate(void)
{
	BeemuInstruction instruction = {.type = BEEMU_INSTRUCTION_TYPE_LOAD_8,
									.params = {.load_params = {
												   .dest = {.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
												   .source = {.pointer = false, .type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 10}}}};
	TEST_ASSERT_EQUAL_UINT8(0, registers->registers[BEEMU_REGISTER_A]);
	execute_instruction(memory, registers, instruction);
	TEST_ASSERT_EQUAL_UINT8(10, registers->registers[BEEMU_REGISTER_A]);
}

/**
 * @brief Test case that covers loading from a register to another.
 */
void test_instruction_load_copy()
{
	BeemuInstruction instruction = generate_load_instruction(
		(BeemuParam){.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_B},
		(BeemuParam){.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
		false);
	TEST_ASSERT_EQUAL_UINT8(0, registers->registers[BEEMU_REGISTER_A]);
	TEST_ASSERT_EQUAL_UINT8(0, registers->registers[BEEMU_REGISTER_B]);
	beemu_registers_write_register_value(registers, (BeemuRegister){.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_A}, 5);
	execute_instruction(memory, registers, instruction);
	TEST_ASSERT_EQUAL_UINT8(5, registers->registers[BEEMU_REGISTER_B]);
}

/**
 * @brief Test pointers for both destination and source.
 */
void test_instruction_load_with_pointers()
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
	TEST_ASSERT_EQUAL_UINT8(0, beemu_memory_read(memory, 10));
	execute_instruction(memory, registers, instruction);
	TEST_ASSERT_EQUAL_UINT8(10, beemu_memory_read(memory, 10));
}

/**
 * @brief Test loads from and to 16 bit registers.
 *
 * These need to be tested seperately because they work weird.
 */
void test_instruction_load_16_bit()
{
	BeemuRegister hl = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_HL};
	BeemuRegister h = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_H};
	BeemuRegister l = {.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_L};
	BeemuInstruction instruction = generate_load_instruction(
		(BeemuParam){.type = BEEMU_PARAM_TYPE_REGISTER_16, .value.register_16 = BEEMU_REGISTER_HL},
		(BeemuParam){.type = BEEMU_PARAM_TYPE_UINT16, .value.value = 0xDF},
		true);
	execute_instruction(memory, registers, instruction);
	TEST_ASSERT_EQUAL_UINT16(0xDF, beemu_registers_read_register_value(registers, hl));
	TEST_ASSERT_EQUAL_UINT8(0xF, beemu_registers_read_register_value(registers, h));
	TEST_ASSERT_EQUAL_UINT8(0xD, beemu_registers_read_register_value(registers, l));
}

// not needed when using generate_test_runner.rb
int main(void)
{
	UNITY_BEGIN();
	TEST_MESSAGE("Starting load instruction tests.\n");
	RUN_TEST(test_instruction_load_immediate);
	RUN_TEST(test_instruction_load_copy);
	RUN_TEST(test_instruction_load_with_pointers);
	RUN_TEST(test_instruction_load_16_bit);
	return UNITY_END();
}