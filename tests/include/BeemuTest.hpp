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
		BeemuRegister hl_register = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_HL};

		void SetUp() override;

		void TearDown() override;
	};
};
