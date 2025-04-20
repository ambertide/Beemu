#include "tokenize_common.h"

uint8_t determine_byte_length_and_cleanup(BeemuInstruction *instruction)
{
	if (instruction->original_machine_code >> 16 == 0xCB)
	{
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

	// These values are bitmasks I created from looking at the GB instruction space.
	// Some of them also include invalid or SYSTEM instructions which should have been handled prior
	// to this.
	if (
		(opcode & 0xCF == 1) || opcode == 0x8 || (opcode & 0xEE == 0xC2) || (opcode & 0xCF == 0xC4) || ((opcode & 0xC0) == 0 && (opcode - 0xC) <= 4))
	{
		instruction->original_machine_code &= 0x00FFFFFF;
		instruction->byte_length = 3;
	}
	else if (
		(
			(opcode < 0x40 || opcode >= 0xC0) && (opcode & 0xF7 == 0x06)) ||
		((opcode >= 0xE0 || (opcode < 0x40 && opcode >= 0x10)) && (opcode & 0xF7 == 0)))
	{
		instruction->original_machine_code >>= 8;
		instruction->original_machine_code &= 0x0000FFFFF;
		instruction->byte_length = 2;
	}
	else
	{
		// FINALLY, otherwise, it is just opcode.
		instruction->original_machine_code >>= 16;
		instruction->original_machine_code &= 0xFF;
		instruction->byte_length = 1;
	}

	return opcode;
}
