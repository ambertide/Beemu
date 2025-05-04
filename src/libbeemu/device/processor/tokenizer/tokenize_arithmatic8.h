/**
 * @file tokenize_arithmatic8.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that contains arithmatic8 instructions,
 * and also the weird instructions on top.
 * @version 0.1
 * @date 2025-05-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <libbeemu/device/primitives/instruction.h>
#include <stdint.h>

#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_ARITHMATIC8_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_ARITHMATIC8_H
#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Differentiate the Arithmatic8 subtype during tokenization stage.
	 *
	 */
	typedef enum BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE {
		/** Emitted when not arithmatic. */
		BEEMU_TOKENIZER_ARITHMATIC8_INVALID_ARITHMATIC8,
		/** Emitted for mainline airthmatic8 inst in 0x80-0xC0 exc. */
		BEEMU_TOKENIZER_ARITHMATIC8_MAINLINE_ARITHMATIC8,
		/** Emitted for INC/DEC r8 inst in 0b(00nn n10n) pattern */
		BEEMU_TOKENIZER_ARITHMATIC8_INC_DEC,
		/** Emitted for direct arithmatic loads post mainline. */
		BEEMU_TOKENIZER_ARITHMATIC8_DIRECT,
		/** Emitted for "weird" instructions of DAA, CPL, SCF and CCF*/
		BEEMU_TOKENIZER_ARITHMATIC8_WEIRD
	} BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE;

	/**
	 * @brief Given a partially initiated instruction tokenize it as a arithmatic8
	 * instruction.
	 *
	 * @param instruction Partially constructed instruction known to be under
	 * Arithmatic8 instruction space.
	 * @param opcode Opcode portion of the instruction for faster checks for some
	 * operations.
	 */
	void tokenize_arithmatic8(BeemuInstruction* instruction, uint8_t opcode);

	/**
	 * @brief Check if the given OPCODE is an arithmatic8 operation and if so
	 * determine its subtype.
	 *
	 * @param opcode Opcode to check against, must not be a CBXX opcode.
	 * @return subtype of the arithmatic operation if one exists.
	 */
	BEEMU_TOKENIZER_ARITHMATIC8_SUBTYPE
	arithmatic8_subtype_if_arithmatic8(uint8_t opcode);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_ARITHMATIC8_H