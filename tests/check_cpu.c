#include <unity.h>
#include <json.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

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
			else if (dir->d_name == "." || dir->d_name == "..")
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
	const json_object *inital_values = json_object_object_get(test, "inital");
	const json_object *final_values = json_object_object_get(test, "final");
}

/**
 * @brief Test a single CPU instruction.
 *
 * @param data_file Data file that contains tests.
 */
void exec_single_instruction(char *data_file)
{
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