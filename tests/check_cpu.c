#include <unity.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/instruction.h>

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
}

void tearDown(void)
{
}

/**
 * @brief Test a single CPU instruction.
 *
 * @param data_file Data file that contains tests.
 */
void test_single_instruction(void)
{
	TEST_MESSAGEF("Starting Test %s\n", "for executor.");
	BeemuInstruction instruction = {.type = BEEMU_INSTRUCTION_TYPE_LOAD_8,
									.params = {.load_params = {
												   .dest = {.pointer = false, .type = BEEMU_PARAM_TYPE_REGISTER_8, .value.register_8 = BEEMU_REGISTER_A},
												   .source = {.pointer = false, .type = BEEMU_PARAM_TYPE_UINT_8, .value.value = 10}}}};
	BeemuMemory *memory = beemu_memory_new(64000);
	BeemuRegisters *registers = beemu_registers_new();
	TEST_ASSERT_EQUAL_UINT8(0, registers->registers[BEEMU_REGISTER_A]);
	execute_instruction(memory, registers, instruction);
	TEST_ASSERT_EQUAL_UINT8(10, registers->registers[BEEMU_REGISTER_A]);
	TEST_MESSAGEF("Test %s Passed.\n", "for executor");
}

// not needed when using generate_test_runner.rb
int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_single_instruction);
	return UNITY_END();
}