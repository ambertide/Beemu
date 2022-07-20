#include <unity.h>
#include <json.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>

#include <string.h>
#include <libbeemu/device/processor.h>
#include <libbeemu/device/memory.h>
#include <libbeemu/device/registers.h>

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
		TEST_MESSAGE(msg);                       \
	}

const int path_name_length = 48;
typedef struct TestFiles
{
	char **test_file_paths;
	int test_file_count;
} TestFiles;

TestFiles cpu_files = {NULL, 0};

TestFiles get_cpu_tests()
{
	int number_of_tests = 0;
	DIR *d;
	struct dirent *dir;
	d = opendir("../libs/gameboy-test-data/cpu_tests/v1");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			number_of_tests++;
		}
		closedir(d);
	}
	char **cpu_test_paths = (char **)malloc((number_of_tests - 2) * sizeof(char *));
	d = opendir("../libs/gameboy-test-data/cpu_tests/v1");
	if (d)
	{
		for (int i = 0; i < number_of_tests; i++)
		{
			dir = readdir(d);
			if (dir == NULL)
			{
				break;
			}
			else if (dir->d_name[0] == '.')
			{
				continue;
			}
			cpu_test_paths[i - 2] = malloc(sizeof(char *) * (path_name_length + 1));
			snprintf(cpu_test_paths[i - 2], path_name_length + 1, "../libs/gameboy-test-data/cpu_tests/v1/%s", dir->d_name);
		}
		closedir(d);
	}
	TestFiles test_files = {cpu_test_paths, number_of_tests - 2};
	return test_files;
}

void setUp(void)
{
	cpu_files = get_cpu_tests();
}

void tearDown(void)
{
	for (int i = 0; i < cpu_files.test_file_count; i++)
	{
		free(cpu_files.test_file_paths[i]);
	}
	free(cpu_files.test_file_paths);
}

/**
 * @brief Execute a single test.
 *
 * @param test
 */
void exec_test_case(json_object *test)
{
	const char *test_name = json_object_get_string(json_object_object_get(test, "name"));
	const json_object *inital_values = json_object_object_get(test, "initial");
	const json_object *initial_cpu_values = json_object_object_get(inital_values, "cpu");
	const json_object *initial_ram_values = json_object_object_get(inital_values, "ram");
	const json_object *final_values = json_object_object_get(test, "final");
	const json_object *final_cpu_values = json_object_object_get(final_values, "cpu");
	const json_object *final_ram_values = json_object_object_get(final_values, "ram");
	BeemuProcessor *processor = beemu_processor_new();
	// First load the device state.
	json_object_object_foreach(initial_cpu_values, key, val)
	{
		const char *register_name = key;
		const char *value = json_object_get_string(val);
		if (strncmp(register_name, "sp", 2) == 0 || strncmp(register_name, "pc", 2) == 0)
		{
			const BeemuRegister_16 register_ = strncmp(register_name, "sp", 2) == 0 ? BEEMU_REGISTER_SP : BEEMU_REGISTER_PC;
			beemu_registers_write_16(processor->registers, register_, (uint16_t)strtol(value, NULL, 0));
		}
		else
		{
			// 8 bit registers
			const BeemuRegister_8 register_ = beemu_get_register_from_letter_8(register_name[0]);
			beemu_registers_write_8(processor->registers, register_, (uint8_t)strtol(value, NULL, 0));
		}
	}
	const int number_of_ram_addresses = json_object_array_length(initial_ram_values);
	int last_written_addr = 0;
	for (int addr_index = 0; addr_index < number_of_ram_addresses; addr_index++)
	{
		const json_object *ram_addr_val = json_object_array_get_idx(initial_ram_values, addr_index);
		const int addr = (uint16_t)strtol(json_object_get_string(json_object_array_get_idx(ram_addr_val, 0)), NULL, 0);
		const int val = (uint8_t)strtol(json_object_get_string(json_object_array_get_idx(ram_addr_val, 1)), NULL, 0);
		beemu_memory_write(processor->memory, addr, val);
		last_written_addr = addr;
	}
	// Add a stop right after the last execution.
	beemu_memory_write(processor->memory, last_written_addr + 1, 0x10);

	// Let the processor run.
	while (processor->processor_state != BEEMU_DEVICE_STOP)
	{
		beemu_processor_run(processor);
	}

	// Test the final states.
	json_object_object_foreach(final_cpu_values, key2, val2)
	{
		const char *register_name = key2;
		const char *expected_str = json_object_get_string(val2);
		if (strncmp(register_name, "sp", 2) == 0 || strncmp(register_name, "pc", 2) == 0)
		{
			const BeemuRegister_16 register_ = strncmp(register_name, "sp", 2) == 0 ? BEEMU_REGISTER_SP : BEEMU_REGISTER_PC;
			const uint16_t value = beemu_registers_read_16(processor->registers, register_);
			const uint16_t expected = (uint16_t)strtol(expected_str, NULL, 0);
			TEST_ASSERT_EQUAL_MESSAGEF(expected, value, "In Test %s, Expected %x but got %x in Register %s", test_name, expected, value, register_name);
		}
		else
		{
			// 8 bit registers
			const BeemuRegister_8 register_ = beemu_get_register_from_letter_8(register_name[0]);
			const uint8_t value = beemu_registers_read_8(processor->registers, register_);
			const uint8_t expected = (uint8_t)strtol(expected_str, NULL, 0);
			TEST_ASSERT_EQUAL_MESSAGEF(expected, value, "In Test %s, Expected %x but got %x in Register %s", test_name, expected, value, register_name);
		}
	}
	const int number_of_ram_addresses_in_final = json_object_array_length(initial_ram_values);
	for (int addr_index = 0; addr_index < number_of_ram_addresses_in_final; addr_index++)
	{
		const json_object *ram_addr_val = json_object_array_get_idx(initial_ram_values, addr_index);
		const int addr = (uint16_t)strtol(json_object_get_string(json_object_array_get_idx(ram_addr_val, 0)), NULL, 0);
		const int val = (uint8_t)strtol(json_object_get_string(json_object_array_get_idx(ram_addr_val, 1)), NULL, 0);
		const uint8_t actual_value = beemu_memory_read(processor->memory, addr);
		TEST_ASSERT_EQUAL_MESSAGEF(val, actual_value, "In Test %s, Expected %x but got %x in Memory Location %x", test_name, val, actual_value, addr);
	}
	beemu_processor_free(processor);
}

/**
 * @brief Test a single CPU instruction.
 *
 * @param data_file Data file that contains tests.
 */
void exec_single_instruction(char *data_file)
{
	TEST_MESSAGEF("Starting Test %s.", data_file);
	json_object *root = json_object_from_file(data_file);
	if (!root)
	{
		char message[100];
		snprintf(message, 70, "Could not find %s", data_file);
		TEST_FAIL_MESSAGE(message);
	}
	// The root containing tests is an array
	const int test_count = json_object_array_length(root);
	for (int test_id = 0; test_id < test_count; test_id++)
	{
		// Iterating through the test cases, execute the cases one by one.
		json_object *test = json_object_array_get_idx(root, test_id);
		exec_test_case(test);
	}
	TEST_MESSAGEF("Test %s Passed.", data_file);
	json_object_put(root);
}

void test_cpu_functionality(void)
{
	for (int i = 0; i < cpu_files.test_file_count; i++)
	{
		exec_single_instruction(cpu_files.test_file_paths[i]);
	}
	json_object *root = json_object_from_file("../libs/gameboy-test-data/cpu_tests/00.json");
	if (!root)
		TEST_ASSERT_TRUE(0);
	json_object_put(root);
	TEST_ASSERT_TRUE(1);
}

// not needed when using generate_test_runner.rb
int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_cpu_functionality);
	return UNITY_END();
}