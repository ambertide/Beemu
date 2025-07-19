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
 * Parse a parameter holding an 8 or 16 bit unsigned value.
 */
uint16_t beemu_resolve_instruction_parameter_unsigned(const BeemuParam *parameter, const BeemuProcessor *processor);

#endif // BEEMU_PARSE_COMMON_H
