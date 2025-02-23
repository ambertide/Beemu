add_executable(
	tests
	executor/test_arithmatic.cpp
	executor/test_load.cpp
	executor/test_jump.cpp
	memory/test_memory.cpp
	tokenizer/test_tokens.cpp
	test_registers.cpp
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
gtest_discover_tests(tests)

add_definitions("-DPATH_TO_TEST_RESOURCES=\"${CMAKE_SOURCE_DIR}/tests/resources\"")
