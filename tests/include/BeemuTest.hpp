#include <libbeemu/device/processor/registers.h>
#include <libbeemu/device/memory.h>
#include <gtest/gtest.h>

#ifndef BEEMU_BEEMU_TEST_HPP
#define BEEMU_BEEMU_TEST_HPP

namespace BeemuTests
{
	class BeemuTestFixture : public ::testing::Test
	{
	protected:
		static BeemuRegister create_register_16(BeemuRegister_16 r)
		{
			BeemuRegister register_;
			register_.type = BEEMU_SIXTEEN_BIT_REGISTER;
			register_.name_of.sixteen_bit_register = r;
			return register_;
		}

		static BeemuRegister create_register_8(BeemuRegister_8 r)
		{
			BeemuRegister register_;
			register_.type = BEEMU_EIGHT_BIT_REGISTER;
			register_.name_of.eight_bit_register = r;
			return register_;
		}

		BeemuMemory *memory;
		BeemuRegisters *registers;
		BeemuRegister hl = BeemuTestFixture::create_register_16(BEEMU_REGISTER_HL);
		BeemuRegister pc = BeemuTestFixture::create_register_16(BEEMU_REGISTER_PC);
		BeemuRegister sp = BeemuTestFixture::create_register_16(BEEMU_REGISTER_SP);
		BeemuRegister a = BeemuTestFixture::create_register_8(BEEMU_REGISTER_A);
		BeemuRegister b = BeemuTestFixture::create_register_8(BEEMU_REGISTER_B);
		BeemuRegister c = BeemuTestFixture::create_register_8(BEEMU_REGISTER_C);
		BeemuRegister d = BeemuTestFixture::create_register_8(BEEMU_REGISTER_D);
		BeemuRegister h = BeemuTestFixture::create_register_8(BEEMU_REGISTER_H);
		BeemuRegister l = BeemuTestFixture::create_register_8(BEEMU_REGISTER_L);

		void SetUp() override;

		void TearDown() override;
	};
};

#endif // BEEMU_BEEMU_TEST_HPP