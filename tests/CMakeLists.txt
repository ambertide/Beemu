add_executable(
	tests
#	executor/test_arithmatic.cpp
#	executor/test_load.cpp
#	executor/test_jump.cpp
	processor/BeemuMemoryTest.cpp
	processor/BeemuRegisterTest.cpp
		tokenizer/test_tokens.cpp
	utilities/BeemuProcessorPreset.cpp
	interpreter/test_command_queue.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/BeemuTest.cpp
)

target_link_libraries(
	tests
	PRIVATE
		GTest::gtest_main
		libbeemu
		nlohmann_json::nlohmann_json

)

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(GoogleTest)
set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
gtest_discover_tests(tests)

add_definitions("-DPATH_TO_TEST_RESOURCES=\"${CMAKE_SOURCE_DIR}/tests/resources\"")
