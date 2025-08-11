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
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add a halt cycle command to a command queue.
 */
void beemu_cq_halt_cycle(BeemuCommandQueue *queue);

/**
 * Add a write register 8 machine command to the command queue.
 */
void beemu_cq_write_reg_8(BeemuCommandQueue *queue, BeemuRegister_8 reg, uint8_t value);

/**
 * Add a write register 16 machine command to the command queue.
 */
void beemu_cq_write_reg_16(BeemuCommandQueue *queue, BeemuRegister_16 reg, uint16_t value);

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
 * @brief Resolve the value of a parameter holding an 8 or 16 bit unsigned value.
 *
 * "Resolve" might get the underlying value directly, in case of a register or a memory
 * address for instance, or may dereference that value and fetch the actual value from
 * memory if @param skip_deref is set.
 *
 * @param parameter Parameter to resolve the value of
 * @param processor Processor context.
 * @param skip_deref Do not reference the pointer
 */
uint16_t beemu_resolve_instruction_parameter_unsigned(const BeemuParam *parameter, const BeemuProcessor *processor, bool skip_deref);

/**
 * Holds two params, typically exploded 16 bit register to two 8 bits.
 */
typedef struct BeemuParamTuple {
	BeemuParam higher;
	BeemuParam lower;
} BeemuParamTuple;

/**
 * Given a "compound" parameter (ie: reg16 or uint16) explode it into two
 * base parameters (ie: reg8, reg8 or uint8, uint8)
 * @param param Parameter to explode.
 * @param processor Processor state use to resolve some parameters (like SP)
 * @return Ad-hoc tuple of two beemu params.
 */
BeemuParamTuple beemu_explode_beemu_param(const BeemuParam *param, const BeemuProcessor *processor);

/**
 * A common operation is to derefence HL and get its value, and issuing a halt.
 * @param queue Queue to write to.
 * @param processor Processor state.
 * @return The value at mem addr [HL]
 */
uint8_t dereference_hl_with_halt(BeemuCommandQueue *queue, const BeemuProcessor *processor);

#ifdef __cplusplus
	}
#endif
#endif // BEEMU_PARSE_COMMON_H
