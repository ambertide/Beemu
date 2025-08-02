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

void beemu_cq_write_memory_through_data_bus(BeemuCommandQueue *queue, const uint16_t memory_address, const uint8_t memory_value)
{
	beemu_cq_write_data_bus(queue, memory_value);
	beemu_cq_write_memory(queue, memory_address, memory_value);
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

/**
 * Check if the node whose pointer is passed is a Data Bus write order.
 * @param node Node to check.
 * @return true if a write order to the db.
 */
bool is_command_node_db_modification(const BeemuCommandQueueNode* node)
{
	return node->current->type == BEEMU_COMMAND_WRITE && node->current->write.target.type == BEEMU_WRITE_TARGET_INTERNAL && node->current->write.target.target.internal_target == BEEMU_INTERNAL_WRITE_TARGET_DATA_BUS;
}

/**
 * Check if the node whose pointer is passed is a Address Bus write order.
 */
bool is_command_node_ab_modification(const BeemuCommandQueueNode* node)
{
	return node->current->type == BEEMU_COMMAND_WRITE && node->current->write.target.type == BEEMU_WRITE_TARGET_INTERNAL && node->current->write.target.target.internal_target == BEEMU_INTERNAL_WRITE_TARGET_ADDRESS_BUS;
}

/**
 * Check if the node is a write order for instruction register
 */
bool is_command_node_ir_modificaiton(const BeemuCommandQueueNode* node)
{
	return node->current->type == BEEMU_COMMAND_WRITE && node->current->write.target.type == BEEMU_WRITE_TARGET_INTERNAL && node->current->write.target.target.internal_target == BEEMU_INTERNAL_WRITE_TARGET_INSTRUCTION_REGISTER;
}

/**
 * Check if the node is a write order for program counter
 */
bool is_command_node_pc_modificaiton(const BeemuCommandQueueNode* node)
{
	return node->current->type == BEEMU_COMMAND_WRITE && node->current->write.target.type == BEEMU_WRITE_TARGET_INTERNAL && node->current->write.target.target.internal_target == BEEMU_INTERNAL_WRITE_TARGET_PROGRAM_COUNTER;
}

bool beemu_cq_has_non_ir_data_bus_modification(const BeemuCommandQueue *queue)
{
	bool was_previous_db_modification = false;
	BeemuCommandQueueNode *node = queue->first;
	while (node != NULL) {
		if (is_command_node_db_modification(node)) {
			was_previous_db_modification = true;
			node = node->next;
			continue;
		}

		if (was_previous_db_modification) {
			if (!is_command_node_ir_modificaiton(node)) {
				// Essentially means there is a non-ir db modification.
				return true;
			}
			// Otherwise there isn't and we continue seeking.
			was_previous_db_modification = false;
		}
		node = node->next;
	}
	return was_previous_db_modification;
}

bool beemu_cq_has_non_pc_addr_bus_modification(const BeemuCommandQueue *queue)
{
	// Mirror of above but with PC and Address Bus
	bool was_previous_ab_modification = false;
	BeemuCommandQueueNode *node = queue->first;
	while (node != NULL) {
		if (is_command_node_ab_modification(node)) {
			was_previous_ab_modification = true;
			node = node->next;
			continue;
		}

		if (was_previous_ab_modification) {
			if (!is_command_node_pc_modificaiton(node)) {
				// Essentially means there is a non-pc ab modification.
				return true;
			}
			// Otherwise there isn't and we continue seeking.
			was_previous_ab_modification = false;
		}
		node = node->next;
	}
	return was_previous_ab_modification;
}