/**
 * @file BeemuProcessorPreset.hpp
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_BEEMU_PROCESSOR_PRESET_HPP
#define BEEMU_BEEMU_PROCESSOR_PRESET_HPP
#include "../include/BeemuProcessorSerializers.hpp"
#include <optional>

struct BeemuProcessorPresetRecord {
	std::string preset;
	BeemuProcessor processor;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuProcessorPresetRecord,
	preset,
	processor);

struct BeemuProcessorPresetFile {
	std::vector<BeemuProcessorPresetRecord> processors;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuProcessorPresetFile,
	processors
	);


namespace BeemuTests {
	class ProcessorPresetNotFoundException final : public std::exception {
	private:
		std::string error_message;
	public:
	    explicit ProcessorPresetNotFoundException(const std::string& error_message) : error_message(error_message) {}
		const char* what() const noexcept override
		{
			return this->error_message.c_str();
		};
	};

	class BeemuProcessorPreset {
	public:
	    /**
	     * Get a saved processor preset from the presets file and access it.
	     *
	     * @throws ProcessorPresetNotFoundException if the preset is not in the file.
	     */
	    explicit BeemuProcessorPreset(const std::string&);
		const BeemuProcessor &processor() const;
	private:
		BeemuProcessor _processor{};
		static std::unordered_map<std::string, BeemuProcessor> processor_cache;
		std::string name;
	    /**
	     * Cache a preset in the test-wide cache
	     * @param preset_name Name of the preset.
	     * @param processor Processor to cache.
	     */
		static void cache_preset(const std::string &preset_name, BeemuProcessor processor);

	    /**
	     * Attempt to get the preset if cached.
	     * @param preset_name Preset name to check the cache for.
	     * @return the preset if it exists
	     */
		static std::optional<BeemuProcessor> get_from_cache(const std::string &preset_name);
		static BeemuProcessor getPreset(const std::string &preset_name);
	};
}

#endif //BEEMU_BEEMU_PROCESSOR_PRESET_HPP
