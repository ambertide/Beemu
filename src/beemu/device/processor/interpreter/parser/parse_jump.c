/**
 * @file parse_jump.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-10-12
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parse_jump.h"

int emit_decode_parameter(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const BeemuInstruction *instruction
	) {
	// +1 because that was incremented for reading the opcode
	const uint16_t previous_pc_value = processor->registers->program_counter + 1;
	beemu_cq_write_pc(queue, previous_pc_value + 1);
	beemu_cq_write_ir(queue, instruction->original_machine_code & 0xFF);
	beemu_cq_halt_cycle(queue);
	if (instruction->byte_length == 2) {
		// JR only has one byte of operand space.
		return 1;
	}
	beemu_cq_write_pc(queue, previous_pc_value + 2);
	beemu_cq_write_ir(queue, (instruction->original_machine_code >> 8) & 0xFF);
	beemu_cq_halt_cycle(queue);
	return 2;
}

/**
 * Check if the processor's state meets a condition
 *
 * @param processor Processor to check
 * @param condition Condition to check against
 * @return true if the porcessor fits the condition, false otherwise.
 */
bool do_processor_meet_condition(const BeemuProcessor *processor, BeemuJumpCondition condition)
{
	switch (condition) {
	case BEEMU_JUMP_IF_NO_CONDITION:
		return true;
	case BEEMU_JUMP_IF_CARRY:
		return beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_C);
	case BEEMU_JUMP_IF_NOT_CARRY:
		return !beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_C);
	case BEEMU_JUMP_IF_ZERO:
		return beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_Z);
	case BEEMU_JUMP_IF_NOT_ZERO:
		return !beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_Z);
	default:
		return false;
	}
}

/**
 * Emit a stack pop, also enable interrupts if set,
 * return the value from the stack.
 */
uint16_t emit_stack_pop(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const bool enable_interrupts
	)
{
	const uint16_t current_stack_pointer = processor->registers->stack_pointer;
	const uint16_t memory_value_at_stack = beemu_memory_read_16(processor->memory, current_stack_pointer);
	beemu_cq_write_reg_16(queue, BEEMU_REGISTER_SP, current_stack_pointer + 1);
	beemu_cq_halt_cycle(queue);
	beemu_cq_write_reg_16(queue, BEEMU_REGISTER_SP, current_stack_pointer + 2);
	if (enable_interrupts) {
		beemu_cq_write_ime(queue, 1);
	}
	beemu_cq_halt_cycle(queue);
	return memory_value_at_stack;
}

/**
 * Push to stack an address, and then update the stack pointer.
 */
void emit_stack_push(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const uint16_t address
)
{
	const uint16_t current_stack_pointer = processor->registers->stack_pointer;
	// We are decrementing so write in order, when read this will be little endian.
	beemu_cq_write_memory(queue, current_stack_pointer, address >> 8);
	beemu_cq_write_reg_16(queue, BEEMU_REGISTER_SP, current_stack_pointer - 1);
	beemu_cq_halt_cycle(queue);
	beemu_cq_write_memory(queue, current_stack_pointer, address & 0xFF);
	beemu_cq_write_reg_16(queue, BEEMU_REGISTER_SP, current_stack_pointer - 2);
	beemu_cq_halt_cycle(queue);
}


void emit_jump(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const uint16_t addr
)
{
	beemu_cq_write_pc(queue, addr);
	beemu_cq_write_ir(queue, beemu_memory_read(processor->memory, addr));
	beemu_cq_halt_cycle(queue);
}

void parse_jump(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const BeemuInstruction *instruction) {
	uint16_t current_pc_location = processor->registers->program_counter + 1;
	BeemuJumpParams params = instruction->params.jump_params;
	if (instruction->byte_length > 1) {
		// Means we have a parameter to decode.
		const int decode_count = emit_decode_parameter(queue, processor, instruction);
		current_pc_location += decode_count;
	}

	const BeemuJumpCondition jump_condition = instruction->params.jump_params.condition;

	if (
		jump_condition != BEEMU_JUMP_IF_NO_CONDITION
		&& !do_processor_meet_condition(processor, jump_condition)) {
		if (instruction->byte_length == 1) {
			// Additional cycle is spent checking condition that would have
			// otherwise been calculated during decoding from memory.
			beemu_cq_halt_cycle(queue);
		}
		// Do not emit anything else if no conditions are met.
		return;
	}

	// This completes the fetch cycle, onto the jump cycle.
	uint16_t jump_location = 0;

	switch (instruction->params.jump_params.type) {
	case BEEMU_JUMP_TYPE_CALL:
	case BEEMU_JUMP_TYPE_RST: {
		// For call and rst we must also emit the stack push commands.
		emit_stack_push(queue, processor, params.param.value.value);
	}
	case BEEMU_JUMP_TYPE_JUMP: {
		if (instruction->params.jump_params.is_relative) {
			// For JR, jump location is relative.
			// Also spends a cycle there to calculate that :)
			beemu_cq_halt_cycle(queue);
			jump_location = current_pc_location + params.param.value.signed_value;
		} else {
			// For both CALL and JUMP the value is written in param.
			jump_location = params.param.value.value;
		}
		break;
	}
	case BEEMU_JUMP_TYPE_RET:
		jump_location = emit_stack_pop(queue, processor, params.enable_interrupts);
		break;
	}

	emit_jump(queue, processor, jump_location);
}
