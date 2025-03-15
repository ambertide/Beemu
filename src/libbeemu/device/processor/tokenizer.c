#include <stdlib.h>
#include <libbeemu/device/processor/tokenizer.h>
#include <libbeemu/device/processor/processor.h>
#include <libbeemu/device/memory.h>
#include <libbeemu/internals/utility.h>

BeemuInstruction *beemu_tokenizer_tokenize(uint16_t instruction)
{
	BeemuInstruction *inst = calloc(1, sizeof(BeemuInstruction));
	inst->type = BEEMU_INSTRUCTION_TYPE_ROT_SHIFT;
	inst->duration_in_clock_cycles = 2;
	inst->original_machine_code = instruction;
	inst->is_16 = false;
	inst->params.rot_shift_params.through_carry = true;
	inst->params.rot_shift_params.operation = BEEMU_ROTATE_OP;
	inst->params.rot_shift_params.direction = BEEMU_ROTATION_DIRECTION_LEFT;
	inst->params.rot_shift_params.target.pointer = false;
	inst->params.rot_shift_params.target.type = BEEMU_PARAM_TYPE_REGISTER_8;
	inst->params.rot_shift_params.target.value.register_8 = BEEMU_REGISTER_B;
	return inst;
};
