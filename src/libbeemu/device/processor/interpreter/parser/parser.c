/**
 * @file parser.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parser.h"
#include "parse_arithmatic.h"
#include "parse_bitwise.h"

/**
 * Every gameboy instruction loads the PC and IR values and sets the
 * address and data bus to those values in their first cycle, emit those
 * load events.
 * @param processor
 * @param instruction
 * @param queue
 */
void emit_m1_commands(const BeemuProcessor *processor, const BeemuInstruction *instruction, BeemuCommandQueue *queue)
{
	const uint32_t omc = instruction->original_machine_code;
	uint8_t opcode = (omc & 0xFF0000) ?  (omc & 0xFF0000) >> 16 : (omc & 0xFF00) >> 8;
	if (!opcode) {
		opcode = (omc & 0xFF);
	}
	uint16_t pc_value = processor->registers->program_counter;
	pc_value++;
	beemu_cq_write_pc(queue, pc_value);
	beemu_cq_write_ir(queue, opcode);
	beemu_cq_halt_cycle(queue);
}

/**
 * CBXX instructions has their ACTUAL opcodes decoded using the PC and the IR.
 * @param processor Processor to decode.
 * @param instruction Instruction to decode
 * @param queue Queue to emit at.
 */
void emit_m2_commands_for_cbxx(const BeemuProcessor *processor, const BeemuInstruction *instruction, BeemuCommandQueue *queue)
{
	const uint32_t omc = instruction->original_machine_code;
	// CBXX is always 2 bytes long, and as such...
	const uint8_t actual_opcode = omc & 0xFF;
	// Iterate the PC so that we can act as if the Gameboy read the instruction opcode.
	uint16_t pc_value = processor->registers->program_counter + 2;
	beemu_cq_write_pc(queue, pc_value);
	beemu_cq_write_ir(queue, actual_opcode);
	beemu_cq_halt_cycle(queue);
}

BeemuCommandQueue *beemu_parser_parse(const BeemuProcessor *processor, const BeemuInstruction *instruction) {
	BeemuCommandQueue *queue = beemu_command_queue_new();
	emit_m1_commands(processor, instruction, queue);
	if (instruction->type == BEEMU_INSTRUCTION_TYPE_BITWISE) {
		// This is a CBXX instruction and therefore must also get its
		// actual OPCODE decoded to IR and PC
		emit_m2_commands_for_cbxx(processor, instruction, queue);
	}
	switch (instruction->type) {
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC:
		parse_arithmatic(queue, processor, instruction);
		break;
	case BEEMU_INSTRUCTION_TYPE_BITWISE:
		parse_bitwise(queue, processor, instruction);
		break;
	default:
		break;
	}
	return queue;
}
