/**
 * @file parse_common.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parse_common.h"

#include <libbeemu/device/primitives/instruction.h>
#include <libbeemu/device/processor/registers.h>
#include <stddef.h>

void beemu_cq_halt_cycle(BeemuCommandQueue *queue)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.is_cycle_terminator = true;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_reg_8(BeemuCommandQueue *queue, const BeemuRegister_8 reg, const uint8_t value)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_REGISTER_8;
	command.write.target.target.register_8 = reg;
	command.write.value.is_16 = false;
	command.write.value.value.byte_value = value;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_reg_16(BeemuCommandQueue *queue, const BeemuRegister_16 reg, const uint16_t value)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_REGISTER_16;
	command.write.target.target.register_16 = reg;
	command.write.value.is_16 = true;
	command.write.value.value.double_value = value;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_flag(BeemuCommandQueue *queue, const BeemuFlag flag, const uint8_t value)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_FLAG;
	command.write.target.target.flag = flag;
	command.write.value.is_16 = false;
	command.write.value.value.byte_value = value;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_ir(BeemuCommandQueue *queue, const uint8_t instruction_opcode)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_INTERNAL;
	command.write.target.target.internal_target = BEEMU_INTERNAL_WRITE_TARGET_INSTRUCTION_REGISTER;
	command.write.value.is_16 = false;
	command.write.value.value.byte_value = instruction_opcode;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_pc(BeemuCommandQueue *queue, uint16_t program_counter_value)
{

	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_INTERNAL;
	command.write.target.target.internal_target = BEEMU_INTERNAL_WRITE_TARGET_PROGRAM_COUNTER;
	command.write.value.is_16 = true;
	command.write.value.value.double_value = program_counter_value;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_memory(BeemuCommandQueue *queue, const uint16_t memory_address, const uint8_t memory_value)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_MEMORY_ADDRESS;
	command.write.target.target.mem_addr = memory_address;
	command.write.value.is_16 = false;
	command.write.value.value.byte_value = memory_value;
	beemu_command_queue_enqueue(queue, &command);
}


uint16_t beemu_resolve_instruction_parameter_unsigned(const BeemuParam *parameter, const BeemuProcessor *processor, bool skip_deref)
{
	switch (parameter->type) {
	case BEEMU_PARAM_TYPE_REGISTER_8: {
		BeemuRegister register_to_read;
		register_to_read.type = BEEMU_EIGHT_BIT_REGISTER;
		register_to_read.name_of.eight_bit_register = parameter->value.register_8;
		const uint8_t register_value = beemu_registers_read_register_value(processor->registers, register_to_read);
		if (!parameter->pointer || skip_deref) {
			// If not pointer, nothing else to do.
			return register_value;
		}
		// Otherwise dereference memory.
		const uint8_t mem_value = beemu_memory_read(processor->memory, register_value);
		return mem_value;
	}
	case BEEMU_PARAM_TYPE_REGISTER_16: {
		BeemuRegister register16_to_read;
		register16_to_read.type = BEEMU_SIXTEEN_BIT_REGISTER;
		register16_to_read.name_of.sixteen_bit_register = parameter->value.register_16;
		const uint16_t register16_value = beemu_registers_read_register_value(processor->registers, register16_to_read);
		if (!parameter->pointer || skip_deref) {
			// If not pointer, nothing else to do.
			return register16_value;
		}
		// Otherwise dereference memory.
		const uint8_t memory_value = beemu_memory_read(processor->memory, register16_value);
		return memory_value;
	}
	case BEEMU_PARAM_TYPE_UINT_8:
	case BEEMU_PARAM_TYPE_UINT16: {
		const uint16_t value = parameter->value.value;
		if (!parameter->pointer || skip_deref) {
			return value;
		}
		return beemu_memory_read(processor->memory, value);
		default:
		return 0;
	}
	}
}

BeemuParamTuple beemu_explode_beemu_param(const BeemuParam *param, const BeemuProcessor *processor)
{
	BeemuParamTuple tuple;
	BeemuRegister_8 register_map[3][2] = {
		{BEEMU_REGISTER_B, BEEMU_REGISTER_C},
		{BEEMU_REGISTER_D, BEEMU_REGISTER_E},
		{BEEMU_REGISTER_H, BEEMU_REGISTER_L}
	};
	tuple.higher.pointer = param->pointer;
	tuple.lower.pointer = param->pointer;
	switch (param->type) {
	case BEEMU_PARAM_TYPE_REGISTER_16: {
		if (param->value.register_16 == BEEMU_REGISTER_SP) {
			// SP is... special.
			// it uses the 16-bit only stack pointer so we should
			// manually explode it.
			tuple.higher.type = BEEMU_PARAM_TYPE_UINT_8;
			tuple.lower.type = BEEMU_PARAM_TYPE_UINT_8;
			tuple.higher.value.value = (processor->registers->stack_pointer & 0xFF00) >> 8;
			tuple.lower.value.value = processor->registers->stack_pointer & 0x00FF;

		} else {
			// Otherwise for compound registers,
			// we just explode them to their constutient parts.
			tuple.higher.type = BEEMU_PARAM_TYPE_REGISTER_8;
			tuple.lower.type = BEEMU_PARAM_TYPE_REGISTER_8;
			tuple.higher.value.register_8 = register_map[param->value.register_16][0];
			tuple.lower.value.register_8 = register_map[param->value.register_16][1];
		}
		break;
	}
	case BEEMU_PARAM_TYPE_UINT16: {
		tuple.higher.type = BEEMU_PARAM_TYPE_UINT_8;
		tuple.lower.type = BEEMU_PARAM_TYPE_UINT_8;
		tuple.higher.value.register_8 = (param->value.value & 0xFF00) >> 8;
		tuple.lower.value.register_8 = param->value.value & 0x00FF;
		break;
	}
	default: {
		tuple.higher.type = param->type;
		tuple.higher.value = param->value;
		tuple.lower = tuple.higher;
		break;
	}
	}
	return tuple;
}

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