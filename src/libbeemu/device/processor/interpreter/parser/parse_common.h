/**
 * @file parse_common.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Used for common parsing functions.
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PARSE_COMMON_H
#define BEEMU_PARSE_COMMON_H
#include "../command.h"
#include <stdint.h>
#include <stdbool.h>
#include <libbeemu/device/processor/processor.h>

/**
 * Add a halt cycle command to a command queue.
 */
void beemu_cq_halt_cycle(BeemuCommandQueue *queue);

/**
 * Add a write register 8 machine command to the command queue.
 */
void beemu_cq_write_reg_8(BeemuCommandQueue *queue, BeemuRegister_8 reg, uint8_t value);

/**
 * Add an internal write to data bus command to the command queue.
 */
void beemu_cq_write_data_bus(BeemuCommandQueue *queue, uint8_t value);

/**
 * Add an internal write to address bus command to the command queue.
 */
void beemu_cq_write_address_bus(BeemuCommandQueue *queue, uint16_t value);

/**
 * Add a flag write command to the command queue.
 */
void beemu_cq_write_flag(BeemuCommandQueue *queue, BeemuFlag flag, uint8_t value);

/**
 * Write an instruction opcode to the instruction register.
 */
void beemu_cq_write_ir(BeemuCommandQueue *queue, uint8_t instruction_opcode);

/**
 * Write a instruction's location to the program counter
 */
void beemu_cq_write_pc(BeemuCommandQueue *queue, uint16_t program_counter_value);

/**
 * Emit a write order for a memory address.
 */
void beemu_cq_write_memory(BeemuCommandQueue *queue, uint16_t memory_address, uint8_t memory_value);

/**
 *  Emit a write order for a memory address, preceded by a write order
 *  of the same value to the data bus.
 */
void beemu_cq_write_memory_through_data_bus(BeemuCommandQueue *queue, uint16_t memory_address, uint8_t memory_value);

/**
 * Check if this command queue has a queued write order for a data bus
 * modification for something OTHER than instruction register.
 * @param queue Queue to check for
 * @return True if a write order for the data bus exists without immediately
 * followed by an IR write.
 */
bool beemu_cq_has_non_ir_data_bus_modification(const BeemuCommandQueue *queue);

/**
 * Check if this command queue has a queued addres bus modification for something
 * other than the Program Counter.
 * @param queue Queue to check for
 * @return True if a write order for the address bus exists without immediately
 * followed by a PC write.
 */
bool beemu_cq_has_non_pc_addr_bus_modification(const BeemuCommandQueue *queue);

/**
 * Parse a parameter holding an 8 or 16 bit unsigned value.
 */
uint16_t beemu_resolve_instruction_parameter_unsigned(const BeemuParam *parameter, const BeemuProcessor *processor);

#endif // BEEMU_PARSE_COMMON_H
