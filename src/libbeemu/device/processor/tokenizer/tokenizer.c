#include "tokenize_arithmatic.h"
#include "tokenize_cbxx.h"
#include "tokenize_common.h"
#include "tokenize_load.h"
#include "tokenize_system.h"
#include "tokenize_jump.h"

#include <libbeemu/device/memory.h>
#include <libbeemu/device/processor/processor.h>
#include <libbeemu/device/processor/tokenizer.h>
#include <libbeemu/internals/utility.h>
#include <stdlib.h>

BeemuInstruction* beemu_tokenizer_tokenize(uint32_t instruction)
{
	BeemuInstruction* inst = calloc(1, sizeof(BeemuInstruction));
	inst->original_machine_code = instruction;
	uint8_t opcode = determine_byte_length_and_cleanup(inst);
	if (tokenize_system(inst, opcode)) {
		return inst;
	}

	if ((inst->byte_length == 2 && opcode == 0xCB) || ((opcode & 0xE7) == 0x07) ) {
		// Parse cb prefix seperately
		// as well as the RLA, RRA, RLCA and RRCA special instructions.
		tokenize_cbxx(inst);
	} else if (load_subtype_if_load(opcode)) {
		tokenize_load(inst, opcode);
	} else if (arithmatic_subtype_if_arithmatic(opcode)) {
		tokenize_arithmatic(inst, opcode);
	} else if (jump_subtype_if_jump(opcode)) {
		tokenize_jump(inst, opcode);
	}

	return inst;
};

void beemu_tokenizer_free_token(BeemuInstruction* token)
{
	free(token);
}