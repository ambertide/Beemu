#include <stdlib.h>
#include <libbeemu/device/processor/tokenizer.h>
#include <libbeemu/device/processor/processor.h>
#include <libbeemu/device/memory.h>
#include <libbeemu/internals/utility.h>

BeemuInstruction beemu_tokenizer_tokenize_new(uint8_t instruction)
{
	BeemuInstruction inst;
	inst.original_machine_code = instruction;
	return inst;
};
