#include <unity.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/instruction.h>

BeemuMemory *memory;
BeemuRegisters *registers;

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
	BeemuInstruction instruction = {.type = BEEMU_INSTRUCTION_TYPE_LOAD_8,
									.params = {.load_params = {
												   .dest = {.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_B},
												   .source = {.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A}}}};
	TEST_ASSERT_EQUAL_UINT8(0, registers->registers[BEEMU_REGISTER_A]);
	TEST_ASSERT_EQUAL_UINT8(0, registers->registers[BEEMU_REGISTER_B]);
	beemu_registers_write_register_value(registers, (BeemuRegister){.type = BEEMU_EIGHT_BIT_REGISTER, .name_of.eight_bit_register = BEEMU_REGISTER_A}, 5);
	execute_instruction(memory, registers, instruction);
	TEST_ASSERT_EQUAL_UINT8(5, registers->registers[BEEMU_REGISTER_B]);
}

// not needed when using generate_test_runner.rb
int main(void)
{
	UNITY_BEGIN();
	TEST_MESSAGE("Starting load instruction tests.\n");
	RUN_TEST(test_instruction_load_immediate);
	RUN_TEST(test_instruction_load_copy);
	return UNITY_END();
}