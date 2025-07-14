/**
 * @file BeemuMachineSerializers.hpp
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Serialization declarations for BeemuProcessor struct using nlohmann::json
 * @version 0.1
 * @date 2025-07-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_BEEMU_MACHINE_SERIALIZERS_HPP
#define BEEMU_BEEMU_MACHINE_SERIALIZERS_HPP
#include "libbeemu/device/processor/processor.h"

#include <libbeemu/device/memory.h>
#include <libbeemu/device/processor/processor.h>
#include <libbeemu/device/processor/registers.h>
#include <nlohmann/json.hpp>
#include <vector>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuRegisters,
	registers,
	flags,
	stack_pointer,
	program_counter);

void to_json(nlohmann::json &json, const BeemuMemory &param)
{
	json["memory_size"] = param.memory_size;
	std::vector<uint8_t> mem_vector(param.memory, param.memory + param.memory_size);
	json["memory"] = mem_vector;
}

void from_json(const nlohmann::json &json, BeemuMemory &param)
{
	json.at("memory_size").get_to(param.memory_size);
	std::vector<uint8_t> memory_vector;
	json.at("memory").get_to(memory_vector);
	// After we copy to vector we are actually supposed to
	// convert it to an array so BeemuMemory can receive it.
	auto *memory = static_cast<uint8_t*>(std::malloc(sizeof(uint8_t) * param.memory_size));
	param.memory = memory;
}

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuProcessorState,
	{{BEEMU_DEVICE_NORMAL, "BEEMU_DEVICE_NORMAL"},
	{BEEMU_DEVICE_HALT, "BEEMU_DEVICE_HALT"},
	{BEEMU_DEVICE_STOP, "BEEMU_DEVICE_STOP"},
	{BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE, "BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE"},
	{BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE, "BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE"}}
	);

inline void to_json(nlohmann::json &json, const BeemuProcessor &param)
{
	json["processor_state"] = param.processor_state;
	json["interrupts_enabled"] = param.interrupts_enabled;
	json["elapsed_clock_cycle"] = param.elapsed_clock_cycle;
	json["memory"] = *param.memory;
	json["registers"] = *param.registers;
}

inline void from_json(const nlohmann::json &json, BeemuProcessor &param)
{
	json.at("processor_state").get_to(param.processor_state);
	json.at("interrupts_enabled").get_to(param.interrupts_enabled);
	json.at("elapsed_clock_cycle").get_to(param.elapsed_clock_cycle);
	const auto beemu_memory = static_cast<BeemuMemory*>(std::malloc(sizeof(BeemuMemory)));
	json.at("memory").get_to(*beemu_memory);
	const auto beemu_registers = static_cast<BeemuRegisters*>(std::malloc(sizeof(BeemuRegisters)));
	json.at("registers").get_to(*beemu_registers);
}


#endif //BEEMU_BEEMU_MACHINE_SERIALIZERS_HPP
