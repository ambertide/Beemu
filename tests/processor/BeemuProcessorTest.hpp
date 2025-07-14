/**
 * @file BeemuProcessorTest.hpp
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_BEEMU_PROCESSOR_TEST_HPP
#define BEEMU_BEEMU_PROCESSOR_TEST_HPP
#include <gtest/gtest.h>


namespace BeemuTest {

	class BeemuProcessorParameterizedTestFixture : public ::testing::TestWithParam<std::pair<std::string, BeemuProcessor>>
	{
		protected:
		void SetUp() override {}
		void TearDown() override {}
	};
}



#endif // BEEMU_BEEMU_PROCESSOR_TEST_HPP
