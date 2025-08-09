#ifndef BEEMU_PARSER_UTILS_TEST_HPP
#define BEEMU_PARSER_UTILS_TEST_HPP

#include <gtest/gtest.h>
#include <libbeemu/device/primitives/register.h>

namespace BeemuTests {
	class BeemuExplodeBeemuParamParameterizedTestFixture : public ::testing::TestWithParam<std::tuple<BeemuRegister_16, std::tuple<BeemuRegister_8, BeemuRegister_8>>> {
	public:
		static  std::vector<std::tuple<BeemuRegister_16, std::tuple<BeemuRegister_8, BeemuRegister_8>>> register_test_cases;
	protected:
		void SetUp() override {};
		void TearDown() override {};
	};

	class BeemuExplodeBeemuParamUint16ParameterizedTestFixture : public ::testing::TestWithParam<std::tuple<uint16_t, std::tuple<uint8_t, uint8_t>>> {
	public:
		static  std::vector<std::tuple<uint16_t, std::tuple<uint8_t, uint8_t>>> uint_test_cases;
	protected:
		void SetUp() override {};
		void TearDown() override {};
	};
}
#endif //BEEMU_PARSER_UTILS_TEST_HPP
