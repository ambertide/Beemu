/**
 * @file parse_bitwise.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-08-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parse_bitwise.h"

/**
 * Calculate the result of a bitwise operation on some values.
 * @param value Value to act on.
 * @param targeted_bit nth msb of the byte will be acted upon
 * @param operation_type Type of the operation to perform
 * @return Operation result (not that how it is interpreted depends on OP)
 */
uint8_t resolve_bitwise_op(const uint8_t value, const uint8_t targeted_bit, const BeemuBitOperation operation_type)
{
	switch (operation_type) {
	case BEEMU_BIT_OP_BIT:
		// This doesn't set anything but returns 1 if nth msb is 1, else 0.
		return (value & (1 << targeted_bit)) >> targeted_bit;
	case BEEMU_BIT_OP_RES:
		// This sets the nth msb to 0
		return value & (0xFF ^ (1 << targeted_bit));
	case BEEMU_BIT_OP_SET:
		// This sets the nth msb to 1
		return value | (1 << targeted_bit);
	}
	return 0;
}

void parse_bitwise(BeemuCommandQueue *queue, const BeemuProcessor *processor, const BeemuInstruction *instruction)
{
	const BeemuBitwiseParams *params = &instruction->params.bitwise_params;
	const bool has_hl_deref = params->target.pointer && params->target.type == BEEMU_PARAM_TYPE_REGISTER_16 && params->target.value.register_16 == BEEMU_REGISTER_HL;
	uint8_t target_value = 0;
	if (has_hl_deref) {
		// Spend a cycle dereferencing the HL and getting the value to the data bus.
		target_value = dereference_hl_with_halt(queue, processor);
	} else {
		// Otherwise this is an 8 bit register, whose value must be taken WITHOUT a halt.
		target_value = beemu_resolve_instruction_parameter_unsigned(&params->target, processor, true);
	}

	// Now, calculate the goddamn thing.
	uint8_t result = resolve_bitwise_op(target_value, params->bit_number, params->operation);

	if (params->operation == BEEMU_BIT_OP_BIT) {
		// This only emits flags, and then quits!
		// Take note that it does not matter if we derefed HL
		// since we do not write back.
		beemu_cq_write_flag(queue, BEEMU_FLAG_Z, result);
		beemu_cq_write_flag(queue, BEEMU_FLAG_N, 0);
		beemu_cq_write_flag(queue, BEEMU_FLAG_H, 1);
		return;
	}

	// Otherwise, what we do kinda does depend on whether or not we are
	// writing to memory, OR to the registers.

	if (has_hl_deref) {
		// If memory, we deref hl and write to it, then halt before moving on to M5
		const uint16_t value_of_hl = beemu_resolve_instruction_parameter_unsigned(&params->target, processor, true);
		beemu_cq_write_memory(queue, value_of_hl, result);
		beemu_cq_halt_cycle(queue);
	} else {
		// Otherwise we instead emit to register directly and run.
		beemu_cq_write_reg_8(queue, params->target.value.register_8, result);
	}
}
