#include "tokenize_arithmatic8.h"
#include "libbeemu/device/primitives/instruction.h"
#include "tokenize_common.h"
#include <assert.h>

BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE
arithmatic8_subtype_if_arithmatic8(uint8_t opcode)
{
	static const BeemuTokenizerSubtypeDifferentiator tests[] = {
		// _INVALID
		{ 0xFF, 0xFF },
		// Mainline check specifically looks at whether or not
		// first nibble is 0x8, 0x9, 0xA or 0xB.
		{ 0xC0, 0x80 },
		{ 0xC6, 0x04 }
	};

	return instruction_subtype_if_of_instruction_type(
	    opcode,
	    tests,
	    BEEMU_TOKENIZER_ARITHMATIC8_INVALID_ARITHMATIC8,
	    BEEMU_TOKENIZER_ARITHMATIC8_INC_DEC);
}

void determine_mainline_arithmatic8_params(
    BeemuInstruction* instruction,
    const uint8_t opcode)
{
	static const BeemuOperation suboperations[] = {
		BEEMU_OP_ADD,
		BEEMU_OP_ADC,
		BEEMU_OP_SUB,
		BEEMU_OP_SBC,
		BEEMU_OP_AND,
		BEEMU_OP_XOR,
		BEEMU_OP_OR,
		BEEMU_OP_CP
	};
	// Once again the registers cycle every 8 instructions.
	const uint8_t src_register_differentiator
	    = instruction->original_machine_code & 0b111;
	// Compare to suboperations, which remain the same 8 times and than cycle through.
	const uint8_t suboperation_differentiator
	    = (instruction->original_machine_code & 0b00111000) >> 3;
	instruction->params.arithmatic_params.operation = suboperations[suboperation_differentiator];
	tokenize_register_param_with_index(
	    &instruction->params.arithmatic_params.source_or_second,
	    src_register_differentiator);
	// And the destination for these is always A, I mean not really,
	// strictly speaking CP does not have a destination, this is why
	// the naming is dest_or_first.
	tokenize_register_param_with_index(
	    &instruction->params.arithmatic_params.dest_or_first,
	    7);
}

void determine_arithmatic8_inc_dec_params(
    BeemuInstruction* instruction,
    const uint8_t opcode)
{
	static const BeemuOperation suboperations[] = {
		// A rather... interseting design decision I had given early on
		// was to encode INC and DEC as add and sub respectively.
		BEEMU_OP_ADD,
		BEEMU_OP_SUB
	};
	const uint8_t operation_selector = opcode & 0x1;
	const uint8_t register_selector = (opcode & 0b00111000) >> 3;
	tokenize_register_param_with_index(&instruction->params.arithmatic_params.dest_or_first, register_selector);
	static const BeemuParam incrementParam = {
		.pointer = false,
		.type = BEEMU_PARAM_TYPE_UINT_8,
		.value.value = 1
	};
	instruction->params.arithmatic_params.source_or_second = incrementParam;
	instruction->params.arithmatic_params.operation = suboperations[operation_selector];
}

// Array used to dispatch to the determine_load8_SUBTYPE_params function
// for a specific BEEMU_TOKENIZER_LOAD8_SUBTYPE, parallel array to the enum
// values
static const determine_param_function_ptr DETERMINE_PARAM_DISPATCH[]
    = {
	      0,
	      &determine_mainline_arithmatic8_params,
	      &determine_arithmatic8_inc_dec_params
      };

void determine_arithmatic8_params(BeemuInstruction* instruction, uint8_t opcode, BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE arithmatic8_params)
{
	determine_param_function_ptr determine_params_func = DETERMINE_PARAM_DISPATCH[arithmatic8_params];
	determine_params_func(instruction, opcode);
}

void determine_arithmatic8_clock_cycle(BeemuInstruction* instruction, BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE operation_subtype)
{
	instruction->duration_in_clock_cycles = 1;
	if (instruction->params.arithmatic_params.source_or_second.pointer) {
		instruction->duration_in_clock_cycles++;
	}
	if (instruction->params.arithmatic_params.dest_or_first.pointer && operation_subtype == BEEMU_TOKENIZER_ARITHMATIC8_INC_DEC) {
		// For INC/DEC a pointer causes a read AND a write.
		// so, extra time spent.
		instruction->duration_in_clock_cycles = 3;
	}
}

void tokenize_arithmatic8(BeemuInstruction* instruction, uint8_t opcode)
{
	const BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE arithmatic8_subtype = arithmatic8_subtype_if_arithmatic8(opcode);
	assert(arithmatic8_subtype != BEEMU_TOKENIZER_ARITHMATIC8_INVALID_ARITHMATIC8);
	instruction->type = BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8;
	determine_arithmatic8_params(instruction, opcode, arithmatic8_subtype);
	determine_arithmatic8_clock_cycle(instruction, arithmatic8_subtype);
}