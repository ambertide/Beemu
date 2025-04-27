#include <stdlib.h>
#include <libbeemu/device/processor/tokenizer.h>
#include <libbeemu/device/processor/processor.h>
#include <libbeemu/device/memory.h>
#include <libbeemu/internals/utility.h>
#include "tokenize_common.h"
#include "tokenize_cbxx.h"
#include "tokenize_load8.h"

BeemuInstruction *beemu_tokenizer_tokenize(uint32_t instruction)
{
	BeemuInstruction *inst = calloc(1, sizeof(BeemuInstruction));
	inst->original_machine_code = instruction;
	uint8_t opcode = determine_byte_length_and_cleanup(inst);
	if (inst->byte_length == 2 && opcode == 0xCB)
	{
		// Parse cb prefix seperately..
		tokenize_cbxx(inst);
	}
	else if (load_subtype_if_load(opcode))
	{
		tokenize_load8(inst, opcode);
	}

	return inst;
};

void beemu_tokenizer_free_token(BeemuInstruction *token)
{
	free(token);
}