/**
 * @file parse_arithmatic.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parse_arithmatic.h"

/**
 * When called with [HL], the M2 cycle is spent fetching the memory value
 * and placing it into the data bus, as well as the HL value itself to the address
 * bus, this function generates commands to do so, as well as returning the [HL] value.
 *
 * @param queue Queue to add the commands to.
 * @param processor Processor in its current state.
 * @return Dereferenced [HL]
 */
uint8_t dereference_hl_with_halt(BeemuCommandQueue *queue, const BeemuProcessor *processor)
{
	// We can directly fetch the HL as the HL writes always occur after this point,
	// no need to seek within the queue.
	BeemuRegister HL;
	HL.type = BEEMU_SIXTEEN_BIT_REGISTER;
	HL.name_of.sixteen_bit_register = BEEMU_REGISTER_HL;
	const uint16_t addr = beemu_registers_read_register_value(processor->registers, HL);
	const uint8_t mem_value = beemu_memory_read(processor->memory, addr);
	// And the halt order.
	beemu_cq_halt_cycle(queue);
	return mem_value;
}

/**
 * Calculate the result of an operation as int32_t so that overflow/underflow won't occur.
 * @return
 */
int32_t resolve_result_wo_overflow(const uint16_t first_value, const uint16_t second_value, const BeemuOperation operation, const uint8_t carry_flag)
{
	switch (operation) {
	case BEEMU_OP_ADD:
		return first_value + second_value;
	case BEEMU_OP_ADC:
		return first_value + second_value + carry_flag;
	case BEEMU_OP_SUB:
		return first_value - second_value;
	case BEEMU_OP_SBC:
		return first_value - second_value - carry_flag;
	case BEEMU_OP_OR:
		return first_value | second_value;
	case BEEMU_OP_XOR:
		return first_value ^ second_value;
	case BEEMU_OP_AND:
		return first_value & second_value;
	case BEEMU_OP_CP:
		return first_value - second_value;
	default:
		return -1;
	}
}

uint8_t resolve_half_carry_for_arithmatic(
    const uint16_t first_value,
    const uint16_t second_value,
    const uint8_t carry_flag,
    const BeemuOperation operation)
{
	switch (operation) {
	case BEEMU_OP_ADD:
		return ((first_value & 0x0F) + (second_value & 0x0F) & 0x10) == 0x10;
	case BEEMU_OP_ADC:
		return (((first_value & 0x0F) + (second_value & 0x0F) + carry_flag) & 0x10) == 0x10;
		;
	case BEEMU_OP_SUB:
	case BEEMU_OP_CP:
		return (((first_value & 0x0F) - (second_value & 0x0F)) & 0x10) == 0x10;
	case BEEMU_OP_SBC:
		return (((first_value & 0x0F) - (second_value & 0x0F) - carry_flag) & 0x10) == 0x10;
	default:
		return 0;
	}
}

/**
 * Insert flag write orders to the queue given the projected and actual result
 * and the executed operation.
 */
void beemu_cq_write_flags(
	BeemuCommandQueue *queue,
	const int32_t would_be_result,
	const uint32_t actual_result,
	const BeemuOperation operation,
	const uint8_t half_carry_flag_value,
	const bool skip_c
	)
{
	beemu_cq_write_flag(queue, BEEMU_FLAG_Z, actual_result == 0);
	beemu_cq_write_flag(queue, BEEMU_FLAG_N, operation == BEEMU_OP_SUB || operation == BEEMU_OP_CP || operation == BEEMU_OP_SBC);
	if (operation == BEEMU_OP_XOR || operation == BEEMU_OP_OR) {
		// XOR and OR specifically set H and C to 0
		beemu_cq_write_flag(queue, BEEMU_FLAG_H, 0);
		if (!skip_c) {
			beemu_cq_write_flag(queue, BEEMU_FLAG_C,  0);
		}
	} else if (operation == BEEMU_OP_AND) {
		// AND is a bit different and set half-carry to 1 but carry to 0
		beemu_cq_write_flag(queue, BEEMU_FLAG_H, 1);
		if (!skip_c) {
			beemu_cq_write_flag(queue, BEEMU_FLAG_C,  0);
		}
	} else {
		// For normal arithmatic operations, we just check if the actual flow overflowed 0x0F for half-carry
		// and 0xFF for carry, or alternativelly for SBC, we check if it underflowed.
		// TODO: Unsure about the behaviour of H Flag for SUB and SBC operations.
		beemu_cq_write_flag(queue, BEEMU_FLAG_H,  half_carry_flag_value);
		if (!skip_c) {
			beemu_cq_write_flag(queue, BEEMU_FLAG_C, would_be_result != actual_result);
		}
	}
}

/**
 * Check if the given parameter is a HL pointer.
 */
bool is_param_hl_ptr(const BeemuParam *param)
{
	return param->pointer && param->type == BEEMU_PARAM_TYPE_REGISTER_16 && param->value.register_16 == BEEMU_REGISTER_HL;
}

/**
 * @brief Check if the parameter holding byte-length values.
 *
 * This can be a register or a memory address, former holds byte-length
 * values if they are an 8-bit register, the latter always holds 8 bits,
 * and so does pointer's, since they POINT to a memory address.
 *
 * @param param Parameter to check
 * @return True if the location indicated by the BeemuParam can hold a
 * 8 bit value.
 */
static bool do_param_hold_byte_length_values(const BeemuParam *param)
{
	if (param->pointer) {
		// Pointers point to memory addresses
		// which always hold a byte.
		return true;
	}

	const bool is_param_8_bit_register = param->type == BEEMU_PARAM_TYPE_REGISTER_8;

	if (is_param_8_bit_register) {
		// Holds 8 bits even if not a pointer.
		return true;
	}

	return false;
}

/**
 * Emit bytecodes that, when executed will write a byte sized result to its destination.
 * @param queue Queue to emit the commands to.
 * @param dst Parameter specifying the destination.
 * @param result Result value to write
 * @param processor BeemuProcessor to resolve the actual values.
 */
void beemu_cq_write_results_u8(
	BeemuCommandQueue *queue,
	const BeemuParam *dst,
	const uint8_t result,
	const BeemuProcessor *processor)
{
	if (dst->pointer) {
		// We will write to memory.
		// We can just use the resolve_instruction_param function to get the value
		// which we now is the mem addr, and then we can emit the memory write.
		const uint16_t memory_addr = beemu_resolve_instruction_parameter_unsigned(dst, processor, true);
		beemu_cq_write_memory(queue, memory_addr, result);
	} else if (dst->type == BEEMU_PARAM_TYPE_REGISTER_8) {
		beemu_cq_write_reg_8(queue, dst->value.register_8, result);
	}
}

bool halts_after_flags(const BeemuInstruction *instruction)
{
	switch (instruction->original_machine_code) {
	case 0x35:
	case 0x34:
		return true;
	default:
		return false;
	}
}

void parse_arithmatic(BeemuCommandQueue *queue, const BeemuProcessor *processor, const BeemuInstruction *instruction)
{
	BeemuArithmaticParams params = instruction->params.arithmatic_params;

	// Resolve parameters
	uint16_t first_value = beemu_resolve_instruction_parameter_unsigned(&params.dest_or_first, processor, false);
	uint16_t second_value = beemu_resolve_instruction_parameter_unsigned(&params.source_or_second, processor, false);
	if (is_param_hl_ptr(&params.source_or_second) || is_param_hl_ptr(&params.dest_or_first)) {
		dereference_hl_with_halt(queue, processor);
	} else {
		second_value = beemu_resolve_instruction_parameter_unsigned(&params.source_or_second, processor, false);
	}

	// Actually calculate the results
	const int32_t operation_result = resolve_result_wo_overflow(
		first_value,
		second_value,
		params.operation,
		beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_C));
	// Half carry is better calculated from the raw params.
	const uint8_t half_carry_result = resolve_half_carry_for_arithmatic(
		first_value,
		second_value,
		beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_C),
		params.operation
	);
	uint32_t actual_result = 0;
	if (do_param_hold_byte_length_values(&params.dest_or_first)) {
		// Then we must convert to UINT8_T since destination holds 8 bits.
		const uint8_t actual_result_size_corrected = operation_result;
		if (params.operation != BEEMU_OP_CP) {
			// Compare operation does not actually modify the contents of the destination.
			beemu_cq_write_results_u8(
				queue,
				&params.dest_or_first,
				actual_result_size_corrected,
				processor);
		}
		actual_result = actual_result_size_corrected;
	}
	// Finally generate write orders for the flag values.
	beemu_cq_write_flags(queue, operation_result, actual_result, params.operation, half_carry_result, instruction->original_machine_code < 0x40);
	if (halts_after_flags(instruction)) {
		beemu_cq_halt_cycle(queue);
	}
}
