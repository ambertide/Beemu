#include "tokenize_system.h"

#include "libbeemu/internals/utility.h"

bool tokenize_system(BeemuInstruction* instruction, uint8_t opcode)
{
	switch (opcode) {
	case 0x00:
		instruction->params.system_op = BEEMU_CPU_OP_NOP;
		break;
	case 0x10:
		instruction->params.system_op = BEEMU_CPU_OP_STOP;
		break;
	case 0x76:
		instruction->params.system_op = BEEMU_CPU_OP_HALT;
		break;
	case 0xF3:
		instruction->params.system_op = BEEMU_CPU_OP_DISABLE_INTERRUPTS;
		break;
	case 0xFB:
		instruction->params.system_op = BEEMU_CPU_OP_ENABLE_INTERRUPTS;
		break;
	default:
		// Otherwise not a system instruction.
		// Do not attempt to parse it.
		return false;
	}
	instruction->type = BEEMU_INSTRUCTION_TYPE_CPU_CONTROL;
	instruction->duration_in_clock_cycles = 1;
	return true;
}