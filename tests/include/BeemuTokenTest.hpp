#include "BeemuTokenSerializers.hpp"
#include <beemu/device/primitives/instruction.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct BeemuJSONEncodedPair
{
	std::string instruction;
	BeemuInstruction token;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuJSONEncodedPair,
	instruction,
	token);

struct BeemuTestJSON
{
	std::vector<BeemuJSONEncodedPair> tokens;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuTestJSON,
	tokens);

bool operator==(const BeemuInstruction &lhs, const BeemuInstruction &rhs)
{
	auto lhs_json = nlohmann::json{lhs};
	auto rhs_json = nlohmann::json{rhs};
	return lhs_json == rhs_json;
}

/**
 * @brief Define how a beemu instruction behaves on a ostream.
 *
 * Apperantly overriding this will allow the GTest to correct
 * output my values, allegedly.
 *
 * @param os
 * @param obj
 * @return std::ostream&
 */
std::ostream &operator<<(std::ostream &os, const BeemuInstruction &obj)
{
	nlohmann::json json_rep(obj);
	// write obj to stream
	return os << json_rep.dump();
}

namespace BeemuTests
{
	/**
	 * @brief Get the tokens to be tested from the encoded test file
	 *
	 * @return std::vector<std::pair<uint16_t, BeemuInstruction>>
	 */
	std::vector<std::pair<uint32_t, BeemuInstruction>> getTokensFromTestFile()
	{
		std::string test_file_path = PATH_TO_TEST_RESOURCES;
		test_file_path += "/tokens.json";
		std::ifstream test_file(test_file_path);
		auto parsed_test_data = nlohmann::json::parse(test_file);
		BeemuTestJSON test_data;
		::from_json(parsed_test_data, test_data);
		std::vector<std::pair<uint32_t, BeemuInstruction>> new_vector;
		for (const BeemuJSONEncodedPair &encoded_pair : test_data.tokens)
		{
			std::stringstream stream{encoded_pair.instruction};
			uint32_t decoded_instruction = 0;
			stream >> std::hex >> decoded_instruction;
			if ((decoded_instruction & 0xFFFFFF00) == 0)
			{
				decoded_instruction <<= 16;
			}
			else if ((decoded_instruction & 0xFFFF0000) == 0)
			{
				decoded_instruction <<= 8;
			}
			auto pair = std::make_pair(decoded_instruction, encoded_pair.token);
			new_vector.push_back(pair);
		}
		return new_vector;
	}
	class BeemuTokenParameterizedTestFixture : public ::testing::TestWithParam<std::pair<uint32_t, BeemuInstruction>>
	{
	protected:
		void SetUp() override {}
		void TearDown() override {}
	};
}