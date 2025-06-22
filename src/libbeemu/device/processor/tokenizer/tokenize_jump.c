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
		// RST Instructions
		{0xC7, 0xC7}
	};

	return instruction_subtype_if_of_instruction_type(
		opcode,
		tests,
		BEEMU_TOKENIZER_JUMP_INVALID_JUMP,
		BEEMU_TOKENIZER_JUMP_RST);
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

