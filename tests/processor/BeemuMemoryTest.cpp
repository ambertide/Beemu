#include <stdbool.h>
#include <gtest/gtest.h>
#include "BeemuMemoryTest.hpp"
namespace BeemuTests
{
	/**
	 * @brief Check if we can initialize memory properly.
	 */
	TEST_F(BeemuMemoryTest, New)
	{
		auto new_memory = beemu_memory_new(20);
		ASSERT_EQ(new_memory->memory_size, 20);
		for (auto i = 0; i < 20; i++)
		{
			ASSERT_EQ(new_memory->memory[i], 0);
		}
		beemu_memory_free(new_memory);
	}

	TEST_F(BeemuMemoryTest, BlockGetSize)
	{
		BeemuMemoryBlock new_memory_block{0, 5};
		uint16_t expected_size = 5;
		ASSERT_EQ(expected_size, beemu_memory_block_get_size(new_memory_block));
	}

	/**
	 * Check if 8 bit write results in change in single mem addr.
	 */
	TEST_F(BeemuMemoryTest, Write8)
	{
		beemu_memory_write(memory, 0xFF, 0xAA);
		ASSERT_EQ(0xAA, memory->memory[0xFF]);
		ASSERT_EQ(0, memory->memory[0xFF - 1]);
		ASSERT_EQ(0, memory->memory[0xFF + 1]);
	}

	/**
	 * Check if 16 bit write results in little endian change in mem addr.
	 */
	TEST_F(BeemuMemoryTest, Write16)
	{
		beemu_memory_write_16(memory, 0xFF, 0xAAFF);
		ASSERT_EQ(0xFF, memory->memory[0xFF]);
		ASSERT_EQ(0xAA, memory->memory[0x100]);
	}

	TEST_F(BeemuMemoryTest, WriteBuffer)
	{
		uint8_t buffer[3] = {0xAA, 0xBB, 0xCC};
		beemu_memory_write_buffer(memory, 0xFF, buffer, 3);
		// Check if buffer stays intact.
		ASSERT_EQ(buffer[0], 0xAA);
		ASSERT_EQ(buffer[1], 0xBB);
		ASSERT_EQ(buffer[2], 0xCC);
		// Check if the memory is successfully copied.
		ASSERT_EQ(memory->memory[0xFF], buffer[0]);
		ASSERT_EQ(memory->memory[0xFF + 1], buffer[1]);
		ASSERT_EQ(memory->memory[0xFF + 2], buffer[2]);
	}

	/**
	 * Check if beemu_memory_write_buffer refuses to write iff
	 * there isn't enough space on memory.
	 */
	TEST_F(BeemuMemoryTest, WriteBufferOverflow)
	{
		uint8_t buffer[3] = {0xAA, 0xBB, 0xCC};
		auto result = beemu_memory_write_buffer(memory, 0xFFFF, buffer, 3);
		ASSERT_FALSE(result);
		// Also check memory remains unwritten.
		ASSERT_EQ(memory->memory[0xFFFE], 0);
		auto result2 = beemu_memory_write_buffer(memory, 0xFFFE, buffer, 3);
		// Check mid value overflow.
		ASSERT_FALSE(result2);
		// Also check memory remains unwritten.
		ASSERT_EQ(memory->memory[0xFFFD], 0);
	}

	/**
	 * Check if read function correctly reads the written data.
	 */
	TEST_F(BeemuMemoryTest, Read8)
	{
		beemu_memory_write(memory, 0xFF, 0xAA);
		ASSERT_EQ(beemu_memory_read(memory, 0xFF), 0xAA);
	}

	/**
	 * Check if read16 function correctly reads written 16 bit data.
	 */
	TEST_F(BeemuMemoryTest, Read16)
	{
		beemu_memory_write_16(memory, 0xFF, 0xAAFF);
		ASSERT_EQ(beemu_memory_read_16(memory, 0xFF), 0xAAFF);
	}

	/**
	 * Check if the buffer read respect little endianness.
	 */
	TEST_F(BeemuMemoryTest, ReadBuffer)
	{
		uint8_t buffer[4];
		beemu_memory_write_16(memory, 0xFF, 0xBBAA);
		beemu_memory_write_16(memory, 0x101, 0xDDCC);
		auto has_read = beemu_memory_read_buffer(memory, 0xFF, buffer, 4);
		// Read to buffer.
		EXPECT_TRUE(has_read);
		EXPECT_EQ(buffer[0], 0xAA);
		EXPECT_EQ(buffer[1], 0xBB);
		EXPECT_EQ(buffer[2], 0xCC);
		EXPECT_EQ(buffer[3], 0xDD);
	}

	/**
	 * Expect beemu_memory_read_buffer to refuse read
	 * if it will overflow.
	 */
	TEST_F(BeemuMemoryTest, ReadBufferOverflow)
	{
		uint8_t buffer[4];
		auto has_read = beemu_memory_read_buffer(memory, 0xFFFE, buffer, 3);
		EXPECT_FALSE(has_read);
	}

	/**
	 * Check if memcpy function works correctly.
	 */
	TEST_F(BeemuMemoryTest, Copy)
	{
		beemu_memory_write_16(memory, 0xFF, 0xA0AF);
		beemu_memory_copy(memory, memory, 0xFF, 0xDD, 2);
		EXPECT_EQ(beemu_memory_read_16(memory, 0xDD), 0xA0AF);
	}
}