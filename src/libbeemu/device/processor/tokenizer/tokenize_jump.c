//
// Created by Elsa on 22.06.2025.
//

#include "tokenize_jump.h"
#include "tokenize_common.h"
#include <assert.h>

BEEMU_TOKENIZER_JUMP_SUBTYPE
jump_subtype_if_jump(uint8_t opcode)
{
	static const BeemuTokenizerSubtypeDifferentiator tests[] = {
		// _INVALID
		{ 0xFF, 0xFF },
		// Conditional JP
		{0xE7, 0xC2},
		// RST Instructions
		{0xC7, 0xC7}
	};

	return instruction_subtype_if_of_instruction_type(
		opcode,
		tests,
		BEEMU_TOKENIZER_JUMP_INVALID_JUMP,
		BEEMU_TOKENIZER_JUMP_RST);
}

void determine_jump_conditional_params(BeemuInstruction *instruction, const uint8_t opcode)
{
	assert(opcode == 0xC2 || opcode == 0xD2 || opcode == 0xCA || opcode == 0xDA);
	instruction->duration_in_clock_cycles = 4;
	BeemuJumpParams params;
	params.type = BEEMU_JUMP_TYPE_JUMP;
	// 5th and 4th MSB can be used to ascertein the conditions.
	const uint8_t condition_selector = (opcode & 0x18) >> 3;
	const BeemuJumpCondition conditions[] = {
		BEEMU_JUMP_IF_NOT_ZERO,
		BEEMU_JUMP_IF_ZERO,
		BEEMU_JUMP_IF_NOT_CARRY,
		BEEMU_JUMP_IF_CARRY
	};
	// Parse the jump conditions and params.
	params.condition = conditions[condition_selector];
	params.enable_interrupts = false;
	params.is_conditional = true;
	params.is_relative = false;
	// Now the jump address.
	const uint16_t jump_address = instruction->original_machine_code & 0xFFFF;
	params.param.pointer = true;
	params.param.type = BEEMU_PARAM_TYPE_UINT16;
	params.param.value.value = jump_address;
	instruction->params.jump_params = params;
}

/**
 * Used to determine the parameters of the RST calls.
 */
void determine_jump_rst_params(BeemuInstruction *instruction, uint8_t opcode)
{
	instruction->duration_in_clock_cycles = 4;
	BeemuJumpParams params;
	// Standard reset parameters.
	params.type = BEEMU_JUMP_TYPE_RST;
	params.condition = BEEMU_JUMP_IF_NO_CONDITION;
	params.enable_interrupts = false;
	params.is_conditional = false;
	params.is_relative = false;
	// This is the jump address.
	params.param.type = BEEMU_PARAM_TYPE_UINT16;
	params.param.pointer = true;
	// The 4th, 5th and 6th variable bits of the opcode
	// together decide which address the processor will jump
	// to (or reset to, hence RST).
	const uint8_t jump_addresses_selector = (opcode & 0x38) >> 3;
	const uint16_t jump_address = jump_addresses_selector * 0x08;
	params.param.value.value = jump_address;
	// Finally set all the parameters.
	instruction->params.jump_params = params;
}

// Array used to dispatch to the determine_load_SUBTYPE_params function
// for a specific BEEMU_TOKENIZER_LOAD_SUBTYPE, parallel array to the enum
// values
static const determine_param_function_ptr DETERMINE_PARAM_DISPATCH[]
	= {
	0,
	&determine_jump_conditional_params,
	&determine_jump_rst_params
};


void tokenize_jump(BeemuInstruction* instruction, uint8_t opcode)
{
	BEEMU_TOKENIZER_JUMP_SUBTYPE subtype = jump_subtype_if_jump(opcode);
	assert(subtype != BEEMU_TOKENIZER_JUMP_INVALID_JUMP);
	instruction->type = BEEMU_INSTRUCTION_TYPE_JUMP;
	// this time we will let the functions themselves determine the
	// clock cycle for brevity.
	determine_param_function_ptr param_func = DETERMINE_PARAM_DISPATCH[subtype];
	param_func(instruction, opcode);
}

