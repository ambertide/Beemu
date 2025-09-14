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
// WRITE CYCLE STEPS:


DEFINE_TERMINAL_STATE(register_write)
{
	assert(ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_16 || ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_8);
	const uint32_t write_value = beemu_resolve_instruction_parameter_unsigned(
		&ctx->ld_params->source,
		ctx->processor,
		false);
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
}

DEFINE_TERMINAL_STATE(write_to_stack) {}

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
	// Mem writes consume an additional cycle.
	beemu_cq_halt_cycle(ctx->queue);
}

/**
 * Start and branch off to the write step.
 */
DEFINE_STATE(write_cycle_start)
{
	const bool write_to_stack = ctx->ld_params->dest.type == BEEMU_PARAM_TYPE_REGISTER_16
		&& ctx->ld_params->dest.pointer
		&& ctx->ld_params->dest.value.register_16 == BEEMU_REGISTER_SP;
	const bool write_to_memory = ctx->ld_params->dest.pointer;
	if (write_to_stack) {
		TRANSITION_TO(write_to_stack);
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
	for (int decoding_nth_byte = 1; decoding_nth_byte < ctx->instruction->byte_length; decoding_nth_byte++ ) {
		const uint8_t next_ir_value = (ctx->instruction->original_machine_code >> (2 << decoding_nth_byte)) & 0xFF;
		// Increment to PC, write the new PC value to IR and Halt.
		beemu_cq_write_pc(ctx->queue, ctx->processor->registers->program_counter + decoding_nth_byte - 1);
		beemu_cq_write_ir(ctx->queue, next_ir_value);
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
	const BeemuRegister_16 target_register = ctx->ld_params->source.value.register_16;
	BeemuRegister reg_query;
	reg_query.type = BEEMU_SIXTEEN_BIT_REGISTER;
	reg_query.name_of.sixteen_bit_register = target_register;
	const uint16_t target_register_value = beemu_registers_read_register_value(
		ctx->processor->registers,
		reg_query);
	const uint16_t new_value = ctx->ld_params->postLoadOperation == BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE
		? target_register_value - 1
		: target_register_value + 1;
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
	const bool has_src_post_load = ctx->ld_params->postLoadOperation != BEEMU_POST_LOAD_NOP
		&& (
			ctx->ld_params->postLoadOperation == BEEMU_POST_LOAD_DECREMENT_INDIRECT_SOURCE
			|| ctx->ld_params->postLoadOperation == BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE);

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