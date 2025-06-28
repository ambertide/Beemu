#include "tokenize_common.h"
#include "bytewidth_lookup_table.gen.h"
#include <assert.h>

uint8_t determine_byte_length_and_cleanup(BeemuInstruction* instruction)
{
	if (instruction->original_machine_code >> 16 == 0xCB) {
		// Toss away the last byte since all CB prefixed
		// instructions are 2 byte long and set the byte length.
		instruction->original_machine_code >>= 8;
		instruction->original_machine_code &= 0x0000FFFF;
		instruction->byte_length = 2;
		return 0xCB;
	}

	// Otherwise the instruction length is variable between 1 to 3.
	// Now since we are inputting the next 3 bytes of memory to an uint32_t
	// integer variable, the 3rd byte from the last will ALWAYS be the OPCODE.
	uint8_t opcode = (instruction->original_machine_code >> 16) & 0xFF;
	// Very cool, secondarily, the opcode of an instruction determines
	// the byte length of the operand and thus the canonical instruction itself.

	// Easiest way to resolve the byte length is of course a lookup table
	// though seperated into two, for brevity.

	uint8_t byte_length = 1;
	if (opcode < 0x40) {
		byte_length = PRE_0X40_BYTE_LENGTH_LOOKUP[opcode];
	} else if (opcode >= 0xC0) {
		byte_length = POST_0XBF_BYTE_LENGTH_LOOKUP[opcode - 0xC0];
	}
	// Otherwise it is always 1.
	// Now we need to act on the data using this information.
	instruction->byte_length = byte_length;

	if (byte_length == 1) {
		instruction->original_machine_code >>= 16;
		instruction->original_machine_code &= 0xFF;
	} else if (byte_length == 2) {
		instruction->original_machine_code >>= 8;
		instruction->original_machine_code &= 0xFFFF;
	} else {
		instruction->original_machine_code &= 0xFFFFFF;
	}

	return opcode;
}

void tokenize_register_param_with_index(BeemuParam* param, uint8_t index)
{
	if (index == 0x06) {
		// This is the case for (HL), the dereferenced HL pointer to the memory.
		param->pointer = true;
		param->type = BEEMU_PARAM_TYPE_REGISTER_16;
		param->value.register_16 = BEEMU_REGISTER_HL;
	} else {
		const static BeemuRegister_8 registers[] = {
			BEEMU_REGISTER_B,
			BEEMU_REGISTER_C,
			BEEMU_REGISTER_D,
			BEEMU_REGISTER_E,
			BEEMU_REGISTER_H,
			BEEMU_REGISTER_L,
			BEEMU_REGISTER_A, // Technically the code should NEVER hit here.
			BEEMU_REGISTER_A
		};
		// Otherwise all 0xCB00-40 block acts on 8 bit register values.
		param->pointer = false; // Not really necessary btw.
		param->type = BEEMU_PARAM_TYPE_REGISTER_8;
		param->value.register_8 = registers[index];
	}
}

void tokenize_register16_param_with_index(
    BeemuParam* param,
    const uint8_t register_index,
    const bool is_pointer,
    const BeemuRegister_16 last_register)
{
	assert(register_index <= 3);
	const BeemuRegister_16 registers[] = {
		BEEMU_REGISTER_BC,
		BEEMU_REGISTER_DE,
		BEEMU_REGISTER_HL,
		last_register
	};
	param->type = BEEMU_PARAM_TYPE_REGISTER_16;
	param->pointer = is_pointer;
	param->value.register_16 = registers[register_index];
}

INSTRUCTION_SUBTYPE instruction_subtype_if_of_instruction_type(
    uint8_t opcode,
    const BeemuTokenizerSubtypeDifferentiator* differentiator_rules,
    INSTRUCTION_SUBTYPE zeroth_instruction_subtype,
    INSTRUCTION_SUBTYPE terminal_instruction_subtype)
{
	for (
	    INSTRUCTION_SUBTYPE instruction_subtype = (zeroth_instruction_subtype + 1);
	    instruction_subtype <= terminal_instruction_subtype;
	    instruction_subtype++) {
		// Test against each possible test and if it returns the expected result
		// we found the relevant subtype.
		BeemuTokenizerSubtypeDifferentiator current_test = differentiator_rules[instruction_subtype];
		if ((opcode & current_test.bitwise_and_operand) == current_test.bitwise_and_expected_result) {
			// This *seems* to be well defined behaviour.
			return instruction_subtype;
		}
	}

	// Otherwise just return the invalid load.
	return zeroth_instruction_subtype;
}

void parse_signed8_param_from_instruction(
	BeemuParam *param,
	const uint32_t original_machine_code)
{
	// For the payload we need to extract the signed component.
	// Yes I am aware there is a memcpy trick in POSIX to do this
	// but I am unsure if that's portable and frankly lowkey tired.
	param->type = BEEMU_PARAM_TYPE_INT_8;
	param->pointer = false;
	uint8_t payload = original_machine_code & 0xFF;
	const uint8_t sign_bit =  payload >> 8;
	if (sign_bit) {
		payload--;
		payload = ~payload;
		param->value.signed_value = (((int8_t) 0) - payload);
	} else {
		// Otherwise the two's complement is itself so we can just.
		param->value.signed_value = (int8_t) 0 + payload;
	}
}
