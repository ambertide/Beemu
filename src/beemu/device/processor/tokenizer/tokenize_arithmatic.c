#include "tokenize_arithmatic.h"
#include "beemu/device/primitives/instruction.h"
#include "tokenize_common.h"
#include <assert.h>

BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE
arithmatic_subtype_if_arithmatic(uint8_t opcode)
{
	static const BeemuTokenizerSubtypeDifferentiator tests[] = {
		// _INVALID
		{ 0xFF, 0xFF },
		// Mainline check specifically looks at whether or not
		// first nibble is 0x8, 0x9, 0xA or 0xB.
		{ 0xC0, 0x80 },
		{ 0xC6, 0x04 },
		{ 0xC7, 0xC6 },
		{ 0xC7, 0x07 },
		// ADD16 block.
		{0xCF, 0x09},
		// INC/DEC16 block.
		{0xC7, 0x03},
		// ADD SP, s8
		{0xFF, 0xE8}
	};

	return instruction_subtype_if_of_instruction_type(
	    opcode,
	    tests,
	    BEEMU_TOKENIZER_ARITHMATIC_INVALID_ARITHMATIC,
	    BEEMU_TOKENIZER_ARITHMATIC16_SP_SIGNED_SUM);
}

void determine_mainline_arithmatic_params(
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

void determine_arithmatic_inc_dec_params(
    BeemuInstruction* instruction,
    const uint8_t opcode)
{
	static const BeemuOperation suboperations[] = {
		// A rather... interseting design decision I had given early on
		// was to encode INC and DEC as add and sub respectively.
		BEEMU_OP_INC,
		BEEMU_OP_DEC
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

/**
 * @brief Used to parse arithmatic operations done directly with n8.
 *
 * @param instruction Instruction token belonging to the isntructino.
 * @param opcode Opcode of the instruction.
 */
void determine_arithmatic_direct_params(
    BeemuInstruction* instruction,
    const uint8_t opcode)
{
	static const BeemuOperation operations[] = {
		BEEMU_OP_ADD,
		BEEMU_OP_ADC,
		BEEMU_OP_SUB,
		BEEMU_OP_SBC,
		BEEMU_OP_AND,
		BEEMU_OP_XOR,
		BEEMU_OP_OR,
		BEEMU_OP_CP
	};
	const uint8_t suboperation_selector = (opcode & 0b00111000) >> 3;
	BeemuOperation operation = operations[suboperation_selector];
	instruction->params.arithmatic_params.operation = operation;
	tokenize_register_param_with_index(
	    &instruction->params.arithmatic_params.dest_or_first,
	    7);
	BeemuParam direct_param = {
		.pointer = false,
		.type = BEEMU_PARAM_TYPE_UINT_8,
		.value = instruction->original_machine_code & 0xFF
	};
	instruction->params.arithmatic_params.source_or_second = direct_param;
}

/**
 * @brief Specifially used to parse DAA, CPL, SCF and CCF
 *
 * @param instruction Instruction partially tokenized
 * @param opcode Opcode
 */
void determine_arithmatic_weird_params(
    BeemuInstruction* instruction,
    uint8_t opcode)
{
	static const BeemuOperation operations[] = {
		BEEMU_OP_DAA,
		BEEMU_OP_CPL,
		BEEMU_OP_SCF,
		BEEMU_OP_CCF
	};
	const uint8_t operation_selector = ((opcode & 0b11000) >> 3);
	instruction->params.arithmatic_params.operation = operations[operation_selector];
	tokenize_register_param_with_index(&instruction->params.arithmatic_params.dest_or_first, 7);
	tokenize_register_param_with_index(&instruction->params.arithmatic_params.source_or_second, 7);
}

void determine_arithmatic16_add16_params(
	BeemuInstruction *instruction,
	uint8_t opcode)
{
	const uint8_t register_selector = (opcode & 0x30) >> 4;
	// the 5th and 6th MSBs determine source registers since, 0x0n, 0x1n, 0x2n and 0x3n's
	// 0, 1, 2, 3 can be used to index a register array.
	tokenize_register16_param_with_index(
		&instruction->params.arithmatic_params.source_or_second,
		register_selector,
		false,
		BEEMU_REGISTER_SP
		);
	// But all the registers write to the HL register.
	tokenize_register16_param_with_index(
		&instruction->params.arithmatic_params.dest_or_first,
		2,
		false,
		BEEMU_REGISTER_SP);
	// And all of them do add.
	// TODO: Check if ADC is possibly a better fit here.
	instruction->params.arithmatic_params.operation = BEEMU_OP_ADD;
}

void determine_arithmatic16_inc_dec_params(
	BeemuInstruction *instruction,
	uint8_t opcode)
{
	static const BeemuOperation operations[] = {
		BEEMU_OP_INC,
		BEEMU_OP_DEC
	};
	const uint8_t operation_selector = (opcode & 0x08) >> 3;
	const uint8_t register_selector = (opcode & 0x30) >> 4;
	// 4th MSB, if one, implies this is a DEC operation, becuause all dec operations in this
	// block is 0xnB vs 0xn3 and so can be differentiated by their fourth bit. (1011 vs 0011)
	instruction->params.arithmatic_params.operation = operations[operation_selector];
	// Meanwhile the 5th and 6th MSBs determine registers since, 0x0n, 0x1n, 0x2n and 0x3n's
	// 0, 1, 2, 3 can be used to index a register array.
	tokenize_register16_param_with_index(
		&instruction->params.arithmatic_params.dest_or_first,
		register_selector,
		false,
		BEEMU_REGISTER_SP
		);
	// Finally whether decrement or increment we use uint8_t value 1 as the second operand.
	instruction->params.arithmatic_params.source_or_second.value.value = 1;
	instruction->params.arithmatic_params.source_or_second.type = BEEMU_PARAM_TYPE_UINT_8;
	instruction->params.arithmatic_params.source_or_second.pointer = false;
}

/**
 * Used to parse arithmatic instruction that adds a signed integer to the stack pointer.
 */
void determine_arithmatic16_sp_signed_sum_params(
	BeemuInstruction *instruction,
	uint8_t opcode)
{
	assert(opcode == 0xE8);
	// This is a very specific instruction.
	instruction->params.arithmatic_params.operation = BEEMU_OP_ADD;
	tokenize_register16_param_with_index(
		&instruction->params.arithmatic_params.dest_or_first,
		3,
		false,
		BEEMU_REGISTER_SP);
	parse_signed8_param_from_instruction(
		&instruction->params.arithmatic_params.source_or_second,
		instruction->original_machine_code);
}

// Array used to dispatch to the determine_load_SUBTYPE_params function
// for a specific BEEMU_TOKENIZER_LOAD_SUBTYPE, parallel array to the enum
// values
static const determine_param_function_ptr DETERMINE_PARAM_DISPATCH[]
    = {
	      0,
	      &determine_mainline_arithmatic_params,
	      &determine_arithmatic_inc_dec_params,
	      &determine_arithmatic_direct_params,
	      &determine_arithmatic_weird_params,
		  &determine_arithmatic16_add16_params,
		  &determine_arithmatic16_inc_dec_params,
		  &determine_arithmatic16_sp_signed_sum_params
      };

void determine_arithmatic_params(BeemuInstruction* instruction, uint8_t opcode, BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE arithmatic_params)
{
	determine_param_function_ptr determine_params_func = DETERMINE_PARAM_DISPATCH[arithmatic_params];
	determine_params_func(instruction, opcode);
}

void determine_arithmatic_clock_cycle(BeemuInstruction* instruction, BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE operation_subtype)
{
	instruction->duration_in_clock_cycles = 1;
	if (instruction->params.arithmatic_params.source_or_second.pointer) {
		instruction->duration_in_clock_cycles++;
	}
	if (instruction->params.arithmatic_params.dest_or_first.pointer && operation_subtype == BEEMU_TOKENIZER_ARITHMATIC_INC_DEC) {
		// For INC/DEC a pointer causes a read AND a write.
		// so, extra time spent.
		instruction->duration_in_clock_cycles = 3;
	}
	if (instruction->byte_length == 2) {
		instruction->duration_in_clock_cycles++;
	}

	if (operation_subtype == BEEMU_TOKENIZER_ARITHMATIC16_INC_DEC) {
		instruction->duration_in_clock_cycles++;
	}

	if (operation_subtype == BEEMU_TOKENIZER_ARITHMATIC16_ADD16) {
		instruction->duration_in_clock_cycles++;
	}

	if (operation_subtype == BEEMU_TOKENIZER_ARITHMATIC16_SP_SIGNED_SUM) {
		instruction->duration_in_clock_cycles = 4;
	}
}

void tokenize_arithmatic(BeemuInstruction* instruction, uint8_t opcode)
{
	const BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE arithmatic_subtype = arithmatic_subtype_if_arithmatic(opcode);
	assert(arithmatic_subtype != BEEMU_TOKENIZER_ARITHMATIC_INVALID_ARITHMATIC);
	instruction->type = BEEMU_INSTRUCTION_TYPE_ARITHMATIC;
	determine_arithmatic_params(instruction, opcode, arithmatic_subtype);
	determine_arithmatic_clock_cycle(instruction, arithmatic_subtype);
}