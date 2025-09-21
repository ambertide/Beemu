/**
 * @file parse_load.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-09-13
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parse_load.h"

#include <assert.h>

// This file is written in the form of a state machine, each step function therefore
// calls another step function before eventually terminating with a single return.
// The state machine is seperated to a FETCH and WRITE cycle, FETCH cycle fetches
// all the values, and WRITE cycle writes them to their load destinations.

typedef struct StateMachineContext {
	BeemuCommandQueue *queue;
	const BeemuProcessor *processor;
	const BeemuInstruction *instruction;
	const BeemuLoadParams *ld_params;
} StateMachineContext;

// Some auxiliary functions to better model state machine
// this might be a terrible idea.
#define DEFINE_STATE(STATE_NAME) void STATE_NAME##_step(const StateMachineContext *ctx)
#define DEFINE_TERMINAL_STATE(STATE_NAME) void STATE_NAME##_step(const StateMachineContext *ctx)
#define TRANSITION_TO(STATE_NAME) STATE_NAME##_step(ctx)
#define START_STATE_MACHINE fetch_cycle_start_step(&ctx)
#define TERMINATE_STATE_MACHINE return
#define RETURN_TO_PREVIOUS_STATE return

// Utility functions
/**
 * Check if a post load operation decrements its target.
 */
bool post_load_increments(const BeemuPostLoadOperation operation)
{
	return operation == BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE
		|| operation == BEEMU_POST_LOAD_INCREMENT_INDIRECT_DESTINATION;
}

/**
 * Check if a post load operation increments its target.
 */
bool post_load_decrements(const BeemuPostLoadOperation operation)
{
	return operation == BEEMU_POST_LOAD_DECREMENT_INDIRECT_SOURCE
		|| operation == BEEMU_POST_LOAD_DECREMENT_INDIRECT_DESTINATION;
}

/**
 * Check if a post load impacts the destination.
 */
bool post_load_impacts_dst(const BeemuPostLoadOperation operation)
{
	return operation == BEEMU_POST_LOAD_DECREMENT_INDIRECT_DESTINATION
		|| operation == BEEMU_POST_LOAD_INCREMENT_INDIRECT_DESTINATION;
}

/**
 * Check if a post load impacts the source.
 */
bool post_load_impacts_src(const BeemuPostLoadOperation operation)
{
	return operation == BEEMU_POST_LOAD_DECREMENT_INDIRECT_SOURCE
		|| operation == BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE;
}

bool has_post_load(const BeemuLoadParams params)
{
	return params.postLoadOperation != BEEMU_POST_LOAD_NOP;
}

/**
 * Check if operation reads from or writes to stack.
 * @param ld_dest_or_src BeemuLoadParam's dest or source
 * @return true if pop or push, false otherwise.
 */
bool is_stack_op(const BeemuParam ld_dest_or_src)
{
	return ld_dest_or_src.type == BEEMU_PARAM_TYPE_REGISTER_16
			&& ld_dest_or_src.pointer
			&& ld_dest_or_src.value.register_16 == BEEMU_REGISTER_SP;
}

// WRITE CYCLE STEPS:
DEFINE_STATE(dst_post_load)
{
	assert(ctx->ld_params->postLoadOperation != BEEMU_POST_LOAD_NOP);
	const int post_load_modifier = post_load_decrements(ctx->ld_params->postLoadOperation) ? -1 : 1;
	// Calculate the LD value for target
	const uint16_t new_target_value = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->dest,
		ctx->processor,
		true) + post_load_modifier;
	if (ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_16) {
		beemu_cq_write_reg_16(
			ctx->queue,
			 ctx->ld_params->dest.value.register_16,
			new_target_value
			);
	} else {
		// Otherwise write to r8
		beemu_cq_write_reg_8(
			ctx->queue,
			 ctx->ld_params->dest.value.register_8,
			new_target_value);
	}

	RETURN_TO_PREVIOUS_STATE;
}

DEFINE_TERMINAL_STATE(register_write)
{
	assert(ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_16 || ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_8);
	if (post_load_impacts_dst(ctx->ld_params->postLoadOperation)) {
		TRANSITION_TO(dst_post_load);
		beemu_cq_halt_cycle(ctx->queue);
	}
	uint32_t write_value = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->source,
		ctx->processor,
		false);

	if (is_stack_op(ctx->ld_params->source)) {
		write_value = beemu_memory_read_16(ctx->processor->memory, ctx->processor->registers->stack_pointer);
	}
	if (ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_8) {
		beemu_cq_write_reg_8(
			ctx->queue,
			ctx->ld_params->dest.value.register_8,
			write_value);
	} else {
		beemu_cq_write_reg_16(
			ctx->queue,
			ctx->ld_params->dest.value.register_16,
			write_value);
	}
	TERMINATE_STATE_MACHINE;
}

DEFINE_TERMINAL_STATE(write_to_stack)
{
	// Stack by definition only holds 16 bit values, they must also be
	// encoded in little endian since we are writing to memory.
	uint16_t stack_ptr = ctx->processor->registers->stack_pointer;
	const uint16_t value_to_push = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->source,
		ctx->processor,
		false);
	const uint8_t msb_value = value_to_push >> 8;
	const uint8_t lsb_value = value_to_push & 0xFF;
	// Write is performed in byte-wise order in reverse.
	beemu_cq_write_reg_16(ctx->queue, BEEMU_REGISTER_SP, --stack_ptr);
	beemu_cq_halt_cycle(ctx->queue);
	beemu_cq_write_memory(ctx->queue, stack_ptr, msb_value);
	beemu_cq_write_reg_16(ctx->queue, BEEMU_REGISTER_SP, --stack_ptr);
	beemu_cq_halt_cycle(ctx->queue);
	beemu_cq_write_memory(ctx->queue, stack_ptr, lsb_value);
	beemu_cq_halt_cycle(ctx->queue);
}

DEFINE_TERMINAL_STATE(write_to_memory)
{
	const uint16_t memory_addr = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->dest,
		ctx->processor,
		true); // SKIP Deref so we can get the full memaddr
	const uint8_t memory_value = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->source,
		ctx->processor,
		false);
	beemu_cq_write_memory(
		ctx->queue,
		memory_addr,
		memory_value
		);

	if (post_load_impacts_dst(ctx->ld_params->postLoadOperation)) {
		TRANSITION_TO(dst_post_load);
	}
	// Mem writes consume an additional cycle.
	beemu_cq_halt_cycle(ctx->queue);
	TERMINATE_STATE_MACHINE;
}

/**
 * Write a double to memory in little endian.
 */
DEFINE_TERMINAL_STATE(write_double_to_memory)
{
	const uint16_t mem_addr = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->dest,
		ctx->processor,
		true);
	const uint16_t value = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->source,
		ctx->processor,
		false);
	const uint8_t lsb = value & 0xFF;
	const uint8_t msb = value >> 8;
	beemu_cq_write_memory(ctx->queue, mem_addr, lsb);
	beemu_cq_halt_cycle(ctx->queue);
	beemu_cq_write_memory(ctx->queue, mem_addr + 1, msb);
	beemu_cq_halt_cycle(ctx->queue);
	TERMINATE_STATE_MACHINE;
}

/**
 * Start and branch off to the write step.
 */
DEFINE_STATE(write_cycle_start)
{
	const bool write_to_stack = is_stack_op(ctx->ld_params->dest);
	const bool write_to_memory = ctx->ld_params->dest.pointer;
	const bool write_double_to_memory = ctx->ld_params->dest.pointer && beemu_param_holds_double(&ctx->ld_params->source);
	if (write_to_stack) {
		TRANSITION_TO(write_to_stack);
	} else if (write_double_to_memory) {
		TRANSITION_TO(write_double_to_memory);
	} else if (write_to_memory) {
		TRANSITION_TO(write_to_memory);
	} else {
		TRANSITION_TO(register_write);
	}
}

// FETCH CYCLE STEPS:

/**
 * Step which fetches the little endian operand from the
 * bytecode and loads it into ALU/IDU
 */
DEFINE_STATE(decode_operand)
{
	const uint16_t operand =
		(ctx->ld_params->source.type == BEEMU_PARAM_TYPE_UINT16 || ctx->ld_params->source.type == BEEMU_PARAM_TYPE_UINT_8)
			? ctx->ld_params->source.value.value
			: ctx->ld_params->dest.value.value;
	for (int decoding_nth_byte = 1; decoding_nth_byte < ctx->instruction->byte_length; decoding_nth_byte++ ) {
		// Below works because we are essentially simulating how a little endian
		// system reads a number from memory while we know the number's correct
		// representation, so we can just read it from the end
		const uint8_t next_ir_value = operand >> (8 * (decoding_nth_byte - 1)) & 0xFF;
		// Increment to PC, write the new PC value to IR and Halt.
		beemu_cq_write_pc(
			ctx->queue,
			ctx->processor->registers->program_counter + (decoding_nth_byte + 1));
		beemu_cq_write_ir(ctx->queue, next_ir_value);
		beemu_cq_halt_cycle(ctx->queue);
	}

	if (ctx->ld_params->source.pointer) {
		// When decoding a pointer as operand, a cycle is spent
		// dereferencing it and storing it on a temporary register.
		beemu_cq_halt_cycle(ctx->queue);
	}
	TRANSITION_TO(write_cycle_start);
}

/**
 * In fetch cycle, emit the step which reads from SP
 * this is special from other cases because we increment
 * SP _slowly_, one cycle at a time.
 */
DEFINE_STATE(read_from_stack)
{
	BeemuRegister sp_reg_query;
	sp_reg_query.type = BEEMU_SIXTEEN_BIT_REGISTER;
	sp_reg_query.name_of.sixteen_bit_register = BEEMU_REGISTER_SP;
	const uint16_t former_sp_value = beemu_registers_read_register_value(
		ctx->processor->registers,
		sp_reg_query);
	beemu_cq_write_reg_16(ctx->queue, BEEMU_REGISTER_SP, former_sp_value + 1);
	beemu_cq_halt_cycle(ctx->queue);
	beemu_cq_write_reg_16(ctx->queue, BEEMU_REGISTER_SP, former_sp_value + 2);
	beemu_cq_halt_cycle(ctx->queue);
	TRANSITION_TO(write_cycle_start);
}

DEFINE_STATE(src_post_load)
{
	// POST LOADS at src only occur at 16 bit registers.
	assert(ctx->ld_params->source.type == BEEMU_PARAM_TYPE_REGISTER_16);
	const uint16_t value = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->source,
		ctx->processor,
		true
		);
	const int modifier = post_load_decrements(ctx->ld_params->postLoadOperation) ? -1 : 1;
	const uint16_t new_value = value + modifier;
	beemu_cq_write_reg_16(
		ctx->queue,
		ctx->ld_params->source.value.register_16,
		new_value);
	beemu_cq_halt_cycle(ctx->queue);
	TRANSITION_TO(write_cycle_start);
}

/**
 * In fetch cycle, emit the step which fetches from memory
 * @param ctx
 */
DEFINE_STATE(fetch_memory)
{
	const bool read_from_stack = ctx->ld_params->source.type == BEEMU_PARAM_TYPE_REGISTER_16
		&& ctx->ld_params->source.pointer
		&& ctx->ld_params->source.value.register_16 == BEEMU_REGISTER_SP;
	const bool has_src_post_load = post_load_impacts_src(ctx->ld_params->postLoadOperation);

	if (read_from_stack) {
		// EMIT SP STEP
		TRANSITION_TO(read_from_stack);
	} else if (has_src_post_load) {
		// Not SP but has post load.
		TRANSITION_TO(src_post_load);
	} else {
		// For memory loads that do not fit post load
		// or post load criteria, typically the memory
		// load takes an extra cycle and then moves to write cycle.
		beemu_cq_halt_cycle(ctx->queue);
		if (ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_UINT16 && !ctx->ld_params->dest.pointer) {
			// If you are writing two memory blocks, then there is an extra halt for reading
			// that.
			beemu_cq_halt_cycle(ctx->queue);
		}
		TRANSITION_TO(write_cycle_start);
	}
}

/**
 * Starting step of the fetch cycle.
 */
DEFINE_STATE(fetch_cycle_start)
{
	const bool has_param_written = ctx->instruction->byte_length > 1;
	const bool fetches_from_memory = ctx->ld_params->source.pointer
		|| ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_UINT16
		|| ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_UINT_8;

	if (has_param_written) {
		TRANSITION_TO(decode_operand);
	} else if (fetches_from_memory) {
		TRANSITION_TO(fetch_memory);
	} else {
		// For registers
		TRANSITION_TO(write_cycle_start);
	}
}


void parse_load(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const BeemuInstruction *instruction)
{
	const BeemuLoadParams *ld_params = &instruction->params.load_params;
	const StateMachineContext ctx = {
		queue,
		processor,
		instruction,
		ld_params
	};

	START_STATE_MACHINE;
}