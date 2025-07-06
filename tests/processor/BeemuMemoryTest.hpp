/**
 * @file BeemuMemoryTest.hpp
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-06
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_BEEMU_MEMORY_TEST_HPP
#define BEEMU_BEEMU_MEMORY_TEST_HPP
#include <libbeemu/device/memory.h>
#include <BeemuTest.hpp>

namespace BeemuTests {
	class BeemuMemoryTest : public ::testing::Test
	{
	protected:
		BeemuMemory *memory = nullptr;

		void SetUp() override
		{
			this->memory = beemu_memory_new(65536);
		}

		void TearDown() override
		{
			beemu_memory_free(this->memory);
		}
	};
}

#endif //BEEMU_BEEMU_MEMORY_TEST_HPP
