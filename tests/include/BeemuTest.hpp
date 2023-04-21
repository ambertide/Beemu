#include <libbeemu/device/processor/registers.h>
#include <libbeemu/device/memory.h>
#include <gtest/gtest.h>

namespace BeemuTests
{
	class BeemuTestFixture : public ::testing::Test
	{
	protected:
		BeemuMemory *memory;
		BeemuRegisters *registers;

		void SetUp() override;

		void TearDown() override;
	};
};
