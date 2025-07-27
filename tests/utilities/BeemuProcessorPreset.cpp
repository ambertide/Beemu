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
#include <iostream>
#include <string>

std::unordered_map<std::string, BeemuProcessor> BeemuTests::BeemuProcessorPreset:: processor_cache {};

BeemuTests::BeemuProcessorPreset::BeemuProcessorPreset(const std::string& name)
{
	this->name = name;
 	this->_processor = getPreset(name);
}

const BeemuProcessor& BeemuTests::BeemuProcessorPreset::processor() const
{
	return this->_processor;
}

void BeemuTests::BeemuProcessorPreset::cache_preset(const std::string& preset_name, BeemuProcessor processor)
{
	processor_cache.insert(std::make_pair(preset_name, processor));
}

std::optional<BeemuProcessor> BeemuTests::BeemuProcessorPreset::get_from_cache(const std::string& preset_name)
{
	std::optional<BeemuProcessor> maybe_processor;
	try {
		auto processor = processor_cache.at(preset_name);
		maybe_processor = processor;
	} catch (const std::out_of_range &e) {
		std::cerr << "Cache miss on key " << preset_name << std::endl;
	}
	return maybe_processor;
}



BeemuProcessor BeemuTests::BeemuProcessorPreset::getPreset(const std::string &preset_name)
{
	if (auto maybe_preset = get_from_cache(preset_name); maybe_preset.has_value()) {
		return maybe_preset.value();
	}
	std::string test_file_path = PATH_TO_TEST_RESOURCES;
	test_file_path += "/processors.json";
	std::ifstream test_file(test_file_path);
	auto parsed_test_data = nlohmann::json::parse(test_file);
	BeemuProcessorPresetFile test_data;
	::from_json(parsed_test_data, test_data);
	std::vector<std::pair<uint32_t, BeemuInstruction>> new_vector;
	for (const BeemuProcessorPresetRecord &preset : test_data.processors)
	{
		//
		cache_preset(preset.preset, preset.processor);
		if (preset.preset == preset_name) {
			return preset.processor;
		}
	}
	throw ProcessorPresetNotFoundException("Unable to find the preset");
}
