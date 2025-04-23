/**
 * @file tokenize_common.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that common tokenizer functions.
 * @version 0.1
 * @date 2025-04-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <libbeemu/device/primitives/instruction.h>
#include <stdint.h>

#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_COMMON_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_COMMON_H
#ifdef __cplusplus
extern "C"
{
#endif
	/**
	 * @brief Given an instruction, determine its byte length and canonize its rep.
	 *
	 * Instructions in gameboy architecture has variable sizes, between 1 and 3 bytes,
	 * therefore we need to cleanup up the clutter (ie: the actual instruction is left padded
	 * with some "noise" that must be deleted on its right.)
	 *
	 * @param instruction Partially constructed instruction whose operands are partially broken
	 * eg: since we always fetch 3 bytes into a uint32_t integer, if our instruction is only two bytes
	 * long, both the least and most significant bytes are actually redundant/noise and must be discarded,
	 * then the instruction must be right padded, or shifted 8 bits to the right.
	 * @return least-significant byte of the instruction, for CBXX this is, of course CB
	 * but for other instructions this is the OPCODE itself.
	 */
	uint8_t determine_byte_length_and_cleanup(BeemuInstruction *instruction);

	/**
	 * @brief Given an index, receive the register param in that index.
	 *
	 * Gameboy consistently enumerates the registers as B, C, D, E, H, L, (HL) and A
	 * for 8 bit instruction param, this function is used to construct a register param
	 * given that order.
	 * @param param param to modify
	 * @param register_index Index of the register param to receive
	 */
	void tokenize_register_param_with_index(BeemuParam *param, const uint8_t register_index);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_COMMON_H