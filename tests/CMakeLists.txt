add_executable(
	tests
	executor/test_arithmatic.cpp
	executor/test_load.cpp
	test_registers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/BeemuTest.cpp
)
target_link_libraries(
	tests
	PRIVATE
		GTest::gtest_main
		libbeemu
)

target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(tests)
