/**
 * @file BeemuCommandSerializer.hpp
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_BEEMU_COMMAND_SERIALIZERS_HPP
#define BEEMU_BEEMU_COMMAND_SERIALIZERS_HPP
#include <nlohmann/json.hpp>
#include "BeemuTokenSerializers.hpp"

inline void from_json(const nlohmann::json &json, BeemuHaltCommand &command)
{
	json.at("is_cycle_terminator").get_to(command.is_cycle_terminator);
	if (!command.is_cycle_terminator) {
		// Optional param.
		json.at("halt_operation").get_to(command.halt_operation);
	}
}

inline void to_json(nlohmann::json &json, const BeemuHaltCommand& command)
{
	json["is_cycle_terminator"] = command.is_cycle_terminator;
	if (!command.is_cycle_terminator) {
		json["halt_operation"] = command.halt_operation;
	}
}

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuWriteTargetType,
	{{BEEMU_WRITE_TARGET_REGISTER_16, "BEEMU_WRITE_TARGET_REGISTER_16"},
		{BEEMU_WRITE_TARGET_REGISTER_8, "BEEMU_WRITE_TARGET_REGISTER_8"},
		{BEEMU_WRITE_TARGET_MEMORY_ADDRESS, "BEEMU_WRITE_TARGET_MEMORY_ADDRESS"},
		{BEEMU_WRITE_TARGET_FLAG, "BEEMU_WRITE_TARGET_FLAG"},
		{BEEMU_WRITE_TARGET_IME, "BEEMU_WRITE_TARGET_IME"},
		{BEEMU_WRITE_TARGET_INTERNAL, "BEEMU_WRITE_TARGET_INTERNAL"}}
	);

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuFlag,
	{
		{BEEMU_FLAG_C, "BEEMU_FLAG_C"},
		{BEEMU_FLAG_H, "BEEMU_FLAG_H"},
		{BEEMU_FLAG_N, "BEEMU_FLAG_N"},
		{BEEMU_FLAG_Z, "BEEMU_FLAG_Z"}
	});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuInternalTargetType,
	{{BEEMU_INTERNAL_WRITE_TARGET_ADDRESS_BUS, "BEEMU_INTERNAL_WRITE_TARGET_ADDRESS_BUS"},
	{BEEMU_INTERNAL_WRITE_TARGET_DATA_BUS, "BEEMU_INTERNAL_WRITE_TARGET_DATA_BUS"}}
);


// Tagged unions require special care.
inline void to_json(nlohmann::json &json, const BeemuWriteTarget param)
{
	json["type"] = param.type;
	switch (param.type) {
	case BEEMU_WRITE_TARGET_REGISTER_16:
		json["target"]["register_16"] = param.target.register_16;
		break;
	case BEEMU_WRITE_TARGET_REGISTER_8:
		json["target"]["register_8"] = param.target.register_8;
		break;
	case BEEMU_WRITE_TARGET_FLAG:
		json["target"]["flag"] = param.target.flag;
		break;
	case BEEMU_WRITE_TARGET_INTERNAL:
		json["target"]["internal_target"] = param.target.internal_target;
		break;
	default:
		throw std::runtime_error("Unknown type encountered for write target.");
	}
}

inline void from_json(const nlohmann::json &json, BeemuWriteTarget &target)
{
	json.at("type").get_to(target.type);
	switch (target.type) {
	case BEEMU_WRITE_TARGET_REGISTER_16:
		json.at("target").at("register_16").get_to(target.target.register_16);
		break;
	case BEEMU_WRITE_TARGET_REGISTER_8:
		json.at("target").at("register_8").get_to(target.target.register_8);
		break;
	case BEEMU_WRITE_TARGET_FLAG:
		json.at("target").at("flag").get_to(target.target.flag);
		break;
	case BEEMU_WRITE_TARGET_INTERNAL:
		json.at("target").at("internal_target").get_to(target.target.internal_target);
		break;
	default:
		throw std::runtime_error("Unknown type encountered for write target.");
	}
}

inline void to_json(nlohmann::json &json, const BeemuWriteValue param)
{
	json["is_16"] = param.is_16;
	if (!param.is_16) {
		json["value"]["byte_value"] = param.value.byte_value;
	} else {
		json["value"]["double_value"] = param.value.double_value;
	}
}

inline void from_json(const nlohmann::json &json, BeemuWriteValue target)
{
	json.at("is_16").get_to(target.is_16);
	if (!target.is_16) {
		json.at("value").at("byte_value").get_to(target.value.byte_value);
	} else {
		json.at("value").at("double_value").get_to(target.value.double_value);
	}
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuWriteCommand,
	target,
	value);

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuCommandType,
	{
		{BEEMU_COMMAND_HALT, "BEEMU_COMMAND_HALT"},
		{BEEMU_COMMAND_WRITE, "BEEMU_COMMAND_WRITE"}
	});

inline void to_json(nlohmann::json &json, const BeemuMachineCommand &command)
{
	json["type"] = command.type;
	if (command.type == BEEMU_COMMAND_WRITE) {
		json["write"] = command.write;
	} else {
		json["halt"] = command.halt;
	}
}

inline void from_json(const nlohmann::json &json, BeemuMachineCommand &command)
{
	json.at("type").get_to(command.type);
	if (command.type == BEEMU_COMMAND_WRITE) {
		json.at("write").get_to(command.write);
	} else {
		json.at("halt").get_to(command.halt);
	}
}

#endif //BEEMU_BEEMU_COMMAND_SERIALIZERS_HPP
