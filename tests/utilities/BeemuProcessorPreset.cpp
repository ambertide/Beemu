/**
 * @file BeemuProcessorPreset.cpp
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "BeemuProcessorPreset.hpp"
#include <fstream>

BeemuTests::BeemuProcessorPreset::BeemuProcessorPreset(const std::string& name)
{
	this->name = name;
 	this->_processor = getPreset(name);
}

const BeemuProcessor& BeemuTests::BeemuProcessorPreset::processor() const
{
	return this->_processor;
}

BeemuProcessor BeemuTests::BeemuProcessorPreset::getPreset(std::string preset_name)
{
	std::string test_file_path = PATH_TO_TEST_RESOURCES;
	test_file_path += "/processors.json";
	std::ifstream test_file(test_file_path);
	auto parsed_test_data = nlohmann::json::parse(test_file);
	BeemuProcessorPresetFile test_data;
	::from_json(parsed_test_data, test_data);
	std::vector<std::pair<uint32_t, BeemuInstruction>> new_vector;
	for (const BeemuProcessorPresetRecord &preset : test_data.processors)
	{
		if (preset.preset == preset_name) {
			return preset.processor;
		}
	}
	throw ProcessorPresetNotFoundException("Unable to find the preset");
}





