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
#include "../include/BeemuMachineSerializers.hpp"

struct BeemuMachineTestPreset {
	std::string machine_state_name;
	BeemuProcessor processor;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuMachineTestPreset,
	machine_state_name,
	processor);

struct BeemuMachineTestFile {
	std::vector<BeemuMachineTestPreset> machine_states;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuMachineTestFile,
	machine_states
	);


namespace BeemuTest {
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
		std::string name;
		static BeemuProcessor getPreset(std::string preset_name);
	};
}

#endif //BEEMU_BEEMU_PROCESSOR_PRESET_HPP
