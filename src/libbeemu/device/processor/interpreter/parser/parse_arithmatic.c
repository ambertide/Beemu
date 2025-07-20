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
uint8_t dereference_hl_and_set_data_address_busses(BeemuCommandQueue *queue, const BeemuProcessor *processor)
{
	// We can directly fetch the HL as the HL writes always occur after this point,
	// no need to seek within the queue.
	BeemuRegister HL;
	HL.type = BEEMU_SIXTEEN_BIT_REGISTER;
	HL.name_of.sixteen_bit_register = BEEMU_REGISTER_HL;
	const uint16_t addr = beemu_registers_read_register_value(processor->registers, HL);
	const uint8_t mem_value = beemu_memory_read(processor->memory, addr);
	// Now write the write orders to the queue.
	beemu_cq_write_address_bus(queue, addr);
	beemu_cq_write_data_bus(queue, mem_value);
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

/**
 * Insert flag write orders to the queue given the projected and actual result
 * and the executed operation.
 */
void beemu_cq_write_flags(BeemuCommandQueue *queue, const int32_t would_be_result, const uint32_t actual_result, BeemuOperation operation)
{
	beemu_cq_write_flag(queue, BEEMU_FLAG_Z, actual_result == 0);
	beemu_cq_write_flag(queue, BEEMU_FLAG_N, operation == BEEMU_OP_SUB || operation == BEEMU_OP_CP || operation == BEEMU_OP_SBC);
	if (operation == BEEMU_OP_XOR || operation == BEEMU_OP_OR) {
		// XOR and OR specifically set H and C to 0
		beemu_cq_write_flag(queue, BEEMU_FLAG_H, 0);
		beemu_cq_write_flag(queue, BEEMU_FLAG_C,  0);
	} else if (operation == BEEMU_OP_AND) {
		// AND is a bit different and set half-carry to 1 but carry to 0
		beemu_cq_write_flag(queue, BEEMU_FLAG_H, 1);
		beemu_cq_write_flag(queue, BEEMU_FLAG_C,  0);
	} else {
		// For normal arithmatic operations, we just check if the actual flow overflowed 0x0F for half-carry
		// and 0xFF for carry, or alternativelly for SBC, we check if it underflowed.
		// TODO: Unsure about the behaviour of H Flag for SUB and SBC operations.
		beemu_cq_write_flag(queue, BEEMU_FLAG_H,  would_be_result != actual_result || actual_result > 0x0F);
		beemu_cq_write_flag(queue, BEEMU_FLAG_C, would_be_result != actual_result);
	}
}

void parse_arithmatic(BeemuCommandQueue *queue, const BeemuProcessor *processor, const BeemuInstruction *instruction)
{
	BeemuArithmaticParams params = instruction->params.arithmatic_params;
	uint16_t first_value = beemu_resolve_instruction_parameter_unsigned(&params.dest_or_first, processor);
	uint16_t second_value = 0;
	if (params.source_or_second.pointer && params.source_or_second.type == BEEMU_PARAM_TYPE_REGISTER_16 && params.source_or_second.value.register_16 == BEEMU_REGISTER_HL) {
		second_value = dereference_hl_and_set_data_address_busses(queue, processor);
	} else {
		second_value = beemu_resolve_instruction_parameter_unsigned(&params.source_or_second, processor);
	}
	const int32_t operation_result = resolve_result_wo_overflow(
		first_value,
		second_value,
		params.operation,
		beemu_registers_flags_get_flag(processor->registers, BEEMU_FLAG_C));
	uint32_t actual_result = 0;
	if (params.dest_or_first.type == BEEMU_PARAM_TYPE_REGISTER_8) {
		// Then we must convert to UINT8_T since destination holds 8 bits.
		const uint8_t actual_result_size_corrected = operation_result;
		if (params.operation != BEEMU_OP_CP) {
			// Compare operation does not actually modify the contents of the register.
			beemu_cq_write_reg_8(queue, params.dest_or_first.value.register_8, actual_result_size_corrected);
		}
		actual_result = actual_result_size_corrected;
	}
	// Finally generate write orders for the flag values.
	beemu_cq_write_flags(queue, operation_result, actual_result, params.operation);
}
