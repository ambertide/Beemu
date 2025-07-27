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
	beemu_cq_write_address_bus(queue, pc_value);
	beemu_cq_write_pc(queue, pc_value);
	beemu_cq_write_data_bus(queue, opcode);
	beemu_cq_write_ir(queue, opcode);
	beemu_cq_halt_cycle(queue);
}

/**
 * Some instructions (Like ADD (HL) or INC (HL)) populate the address and data busses with
 * non-PC or IR values, and these must be put back to their respective busses before the execution
 * finishes.
 * @param processor Processor state.
 * @param instruction Instruction being parsed.
 * @param queue Pointer to so far populated queue.
 */
void emit_pc_ir_normalise_commands(const BeemuProcessor *processor, const BeemuInstruction *instruction, BeemuCommandQueue *queue)
{
	if (beemu_cq_has_non_pc_addr_bus_modification(queue)) {
		// First normalize the address bus with the PC value.
		uint16_t pc_value = processor->registers->program_counter;
		pc_value++;
		beemu_cq_write_address_bus(queue, pc_value);
	}

	if (beemu_cq_has_non_ir_data_bus_modification(queue)) {
		// Then (or possibly first since maybe PC was not modified!) normalize
		// the data bus with the IR value.
		const uint32_t omc = instruction->original_machine_code;
		uint8_t opcode = (omc & 0xFF0000) ?  (omc & 0xFF0000) >> 16 : (omc & 0xFF00) >> 8;
		if (!opcode) {
			opcode = (omc & 0xFF);
		}
		beemu_cq_write_data_bus(queue, opcode);
	}
}

BeemuCommandQueue *beemu_parser_parse(const BeemuProcessor *processor, const BeemuInstruction *instruction) {
	BeemuCommandQueue *queue = beemu_command_queue_new();
	emit_m1_commands(processor, instruction, queue);
	switch (instruction->type) {
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC:
		parse_arithmatic(queue, processor, instruction);
		break;
	default:
		break;
	}
	emit_pc_ir_normalise_commands(processor, instruction, queue);
	return queue;
}
