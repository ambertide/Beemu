#include <libbeemu/device/processor/registers.h>
#include <libbeemu/device/primitives/instruction.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuRegister_8, {{BEEMU_REGISTER_A, "BEEMU_REGISTER_A"},
					  {BEEMU_REGISTER_B, "BEEMU_REGISTER_B"},
					  {BEEMU_REGISTER_C, "BEEMU_REGISTER_C"},
					  {BEEMU_REGISTER_D, "BEEMU_REGISTER_D"},
					  {BEEMU_REGISTER_E, "BEEMU_REGISTER_E"},
					  {BEEMU_REGISTER_H, "BEEMU_REGISTER_H"},
					  {BEEMU_REGISTER_L, "BEEMU_REGISTER_L"}});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuRegister_16, {{BEEMU_REGISTER_BC, "BEEMU_REGISTER_BC"},
					   {BEEMU_REGISTER_DE, "BEEMU_REGISTER_DE"},
					   {BEEMU_REGISTER_HL, "BEEMU_REGISTER_HL"},
					   {BEEMU_REGISTER_AF, "BEEMU_REGISTER_AF"},
					   {BEEMU_REGISTER_M, "BEEMU_REGISTER_M"},
					   {BEEMU_REGISTER_PC, "BEEMU_REGISTER_PC"},
					   {BEEMU_REGISTER_SP, "BEEMU_REGISTER_SP"}});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuInstructionType, {{BEEMU_INSTRUCTION_TYPE_LOAD_8, "BEEMU_INSTRUCTION_TYPE_LOAD_8"},
						   {BEEMU_INSTRUCTION_TYPE_LOAD_16, "BEEMU_INSTRUCTION_TYPE_LOAD_16"},
						   {BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8, "BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8"},
						   {BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16, "BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16"},
						   {BEEMU_INSTRUCTION_TYPE_ROT_SHIFT, "BEEMU_INSTRUCTION_TYPE_ROT_SHIFT"},
						   {BEEMU_INSTRUCTION_TYPE_BITWISE, "BEEMU_INSTRUCTION_TYPE_BITWISE"},
						   {BEEMU_INSTRUCTION_TYPE_CPU_CONTROL, "BEEMU_INSTRUCTION_TYPE_CPU_CONTROL"},
						   {BEEMU_INSTRUCTION_TYPE_JUMP, "BEEMU_INSTRUCTION_TYPE_JUMP"}});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuParamType, {{BEEMU_PARAM_TYPE_REGISTER_8, "BEEMU_PARAM_TYPE_REGISTER_8"},
					 {BEEMU_PARAM_TYPE_REGISTER_16, "BEEMU_PARAM_TYPE_REGISTER_16"},
					 {BEEMU_PARAM_TYPE_UINT_8, "BEEMU_PARAM_TYPE_UINT_8"},
					 {BEEMU_PARAM_TYPE_UINT16, "BEEMU_PARAM_TYPE_UINT16"}});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuWriteLength, {
						  {BEEMU_WRITE_LENGTH_8, "BEEMU_WRITE_LENGTH_8"},
						  {BEEMU_WRITE_LENGTH_16, "BEEMU_WRITE_LENGTH_16"},
					  });

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuOperation, {{BEEMU_OP_ADD, "BEEMU_OP_ADD"},
					 {BEEMU_OP_SUB, "BEEMU_OP_SUB"},
					 {BEEMU_OP_AND, "BEEMU_OP_AND"},
					 {BEEMU_OP_OR, "BEEMU_OP_OR"},
					 {BEEMU_OP_CP, "BEEMU_OP_CP"},
					 {BEEMU_OP_XOR, "BEEMU_OP_XOR"}});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuRotShiftOp, {{BEEMU_ROTATE_OP, "BEEMU_ROTATE_OP"},
					  {BEEMU_SHIFT_ARITHMATIC_OP, "BEEMU_SHIFT_ARITHMATIC_OP"},
					  {BEEMU_SWAP_OP, "BEEMU_SWAP_OP"},
					  {BEEMU_SHIFT_LOGICAL_OP, "BEEMU_SHIFT_LOGICAL_OP"}});

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuRotShiftDirection, {{BEMU_LEFT_DIRECTION, "BEMU_LEFT_DIRECTION"},
							 {BEEMU_RIGHT_DIRECTION, "BEEMU_RIGHT_DIRECTION"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuBitOperation, {{BEEMU_BIT_OP_BIT, "BEEMU_BIT_OP_BIT"},
						{BEEMU_BIT_OP_SET, "BEEMU_BIT_OP_SET"},
						{BEEMU_BIT_OP_RES, "BEEMU_BIT_OP_RES"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuJumpCondition, {{BEEMU_JUMP_IF_NO_CONDITION, "BEEMU_JUMP_IF_NO_CONDITION"},
						 {BEEMU_JUMP_IF_CARRY, "BEEMU_JUMP_IF_CARRY"},
						 {BEEMU_JUMP_IF_NOT_CARRY, "BEEMU_JUMP_IF_NOT_CARRY"},
						 {BEEMU_JUMP_IF_ZERO, "BEEMU_JUMP_IF_ZERO"},
						 {BEEMU_JUMP_IF_NOT_ZERO, "BEEMU_JUMP_IF_NOT_ZERO"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuJumpType, {{BEEMU_JUMP_TYPE_JUMP, "BEEMU_JUMP_TYPE_JUMP"},
					{BEEMU_JUMP_TYPE_CALL, "BEEMU_JUMP_TYPE_CALL"},
					{BEEMU_JUMP_TYPE_RET, "BEEMU_JUMP_TYPE_RET"},
					{BEEMU_JUMP_TYPE_RST, "BEEMU_JUMP_TYPE_RST"}})

NLOHMANN_JSON_SERIALIZE_ENUM(
	BeemuSystemOperation, {{BEEMU_CPU_OP_XOR_CARRY_FLAG, "BEEMU_CPU_OP_XOR_CARRY_FLAG"},
						   {BEEMU_CPU_OP_SET_CARRY_FLAG, "BEEMU_CPU_OP_SET_CARRY_FLAG"},
						   {BEEMU_CPU_OP_NOP, "BEEMU_CPU_OP_NOP"},
						   {BEEMU_CPU_OP_HALT, "BEEMU_CPU_OP_HALT"},
						   {BEEMU_CPU_OP_STOP, "BEEMU_CPU_OP_STOP"},
						   {BEEMU_CPU_OP_DISABLE_INTERRUPTS, "BEEMU_CPU_OP_DISABLE_INTERRUPTS"},
						   {BEEMU_CPU_OP_ENABLE_INTERRUPTS, "BEEMU_CPU_OP_ENABLE_INTERRUPTS"}})

/** SERIALIZE BEEMU PARAM */
void to_json(nlohmann::json &json, const BeemuParam &param)
{
	json["pointer"] = param.pointer;
	json["write_length"] = param.write_length;
	json["type"] = param.type;

	switch (param.type)
	{
	case BEEMU_PARAM_TYPE_REGISTER_8:
		json["value"]["register_8"] = param.value.register_8;
		break;
	case BEEMU_PARAM_TYPE_REGISTER_16:
		json["value"]["register_16"] = param.value.register_16;
		break;
	case BEEMU_PARAM_TYPE_UINT_8:
	case BEEMU_PARAM_TYPE_UINT16:
		json["value"]["value"] = param.value.value;
		break;
	case BEEMU_PARAM_TYPE_INT_8:
		json["value"]["signed_value"] = param.value.signed_value;
		break;
	}
}

void from_json(const nlohmann::json &json, BeemuParam &param)
{
	json.at("pointer").get_to(param.pointer);
	json.at("write_length").get_to(param.write_length);
	json.at("type").get_to(param.type);
	switch (param.type)
	{
	case BEEMU_PARAM_TYPE_REGISTER_8:
		json.at("value").at("register_8").get_to(param.value.register_8);
		break;
	case BEEMU_PARAM_TYPE_REGISTER_16:
		json.at("value").at("register_16").get_to(param.value.register_16);
		break;
	case BEEMU_PARAM_TYPE_UINT_8:
	case BEEMU_PARAM_TYPE_UINT16:
		json.at("value").at("value").get_to(param.value.value);
		break;
	case BEEMU_PARAM_TYPE_INT_8:
		json.at("value").at("signed_value").get_to(param.value.signed_value);
		break;
	}
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuLoadParams,
	source,
	dest);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuArithmaticParams,
	operation,
	dest,
	source);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuRotShiftParams,
	through_carry,
	operation,
	direction,
	target);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuBitwiseParams,
	operation,
	bit_number,
	target);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	BeemuJumpParams,
	is_conditional,
	is_relative,
	enable_interrupts,
	condition,
	param);

// Serialize BeemuInstruction
void to_json(nlohmann::json &json, const BeemuInstruction &inst)
{
	json["original_machine_code"] = inst.original_machine_code;
	json["duration_in_clock_cycles"] = inst.duration_in_clock_cycles;
	json["type"] = inst.type;
	switch (inst.type)
	{
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8:
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16:
		json["params"]["aritmatic_params"] = inst.params.arithmatic_params;
		break;
	case BEEMU_INSTRUCTION_TYPE_BITWISE:
		json["params"]["bitwise_params"] = inst.params.bitwise_params;
		break;
	case BEEMU_INSTRUCTION_TYPE_CPU_CONTROL:
		json["params"]["system_op"] = inst.params.system_op;
		break;
	case BEEMU_INSTRUCTION_TYPE_JUMP:
		json["params"]["jump_params"] = inst.params.jump_params;
		break;
	case BEEMU_INSTRUCTION_TYPE_LOAD_8:
	case BEEMU_INSTRUCTION_TYPE_LOAD_16:
		json["params"]["load_params"] = inst.params.load_params;
		break;
	case BEEMU_INSTRUCTION_TYPE_ROT_SHIFT:
		json["params"]["rot_shift_params"] = inst.params.rot_shift_params;
		break;
	}
}

void from_json(const nlohmann::json &json, BeemuInstruction &inst)
{
	json.at("original_machine_code").get_to(inst.original_machine_code);
	json.at("duration_in_clock_cycles").get_to(inst.duration_in_clock_cycles);
	json.at("type").get_to(inst.type);
	switch (inst.type)
	{
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8:
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16:
		json.at("params").at("aritmatic_params").get_to(inst.params.arithmatic_params);
		break;
	case BEEMU_INSTRUCTION_TYPE_BITWISE:
		json.at("params").at("bitwise_params").get_to(inst.params.bitwise_params);
		break;
	case BEEMU_INSTRUCTION_TYPE_CPU_CONTROL:
		json.at("params").at("system_op").get_to(inst.params.system_op);
		break;
	case BEEMU_INSTRUCTION_TYPE_JUMP:
		json.at("params").at("jump_params").get_to(inst.params.jump_params);
		break;
	case BEEMU_INSTRUCTION_TYPE_LOAD_8:
	case BEEMU_INSTRUCTION_TYPE_LOAD_16:
		json.at("params").at("load_params").get_to(inst.params.load_params);
		break;
	case BEEMU_INSTRUCTION_TYPE_ROT_SHIFT:
		json.at("params").at("rot_shift_params").get_to(inst.params.rot_shift_params);
		break;
	}
}

namespace BeemuTests
{
	class BeemuTokenTest : public ::testing::Test
	{
	protected:
		void SetUp() override {}
		void TearDown() override {}
	};
}