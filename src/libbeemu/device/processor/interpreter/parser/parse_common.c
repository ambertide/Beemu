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

void beemu_cq_write_data_bus(BeemuCommandQueue *queue, const uint8_t value)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_INTERNAL;
	command.write.target.target.internal_target = BEEMU_INTERNAL_WRITE_TARGET_DATA_BUS;
	command.write.value.is_16 = false;
	command.write.value.value.byte_value = value;
	beemu_command_queue_enqueue(queue, &command);
}

void beemu_cq_write_address_bus(BeemuCommandQueue *queue, const uint16_t value)
{
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_WRITE;
	command.write.target.type = BEEMU_WRITE_TARGET_INTERNAL;
	command.write.target.target.internal_target = BEEMU_INTERNAL_WRITE_TARGET_ADDRESS_BUS;
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


uint16_t beemu_resolve_instruction_parameter_unsigned(const BeemuParam *parameter, const BeemuProcessor *processor)
{
	switch (parameter->type) {
	case BEEMU_PARAM_TYPE_REGISTER_8: {
		BeemuRegister register_to_read;
		register_to_read.type = BEEMU_EIGHT_BIT_REGISTER;
		register_to_read.name_of.eight_bit_register = parameter->value.register_8;
		const uint8_t register_value = beemu_registers_read_register_value(processor->registers, register_to_read);
		if (!parameter->pointer) {
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
		if (!parameter->pointer) {
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
		if (!parameter->pointer) {
			return value;
		}
		return beemu_memory_read(processor->memory, value);
		default:
		return 0;
	}
	}
}
