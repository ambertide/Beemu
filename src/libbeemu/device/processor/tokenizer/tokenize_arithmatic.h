/**
 * @file tokenize_arithmatic.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that contains arithmatic instructions,
 * and also the weird instructions on top.
 * @version 0.1
 * @date 2025-05-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <libbeemu/device/primitives/instruction.h>
#include <stdint.h>

#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_ARITHMATIC_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_ARITHMATIC_H
#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Differentiate the arithmatic subtype during tokenization stage.
	 *
	 */
	typedef enum BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE {
		/** Emitted when not arithmatic. */
		BEEMU_TOKENIZER_ARITHMATIC_INVALID_ARITHMATIC,
		/** Emitted for mainline airthmatic8 inst in 0x80-0xC0 exc. */
		BEEMU_TOKENIZER_ARITHMATIC_MAINLINE_ARITHMATIC,
		/** Emitted for INC/DEC r8 inst in 0b(00nn n10n) pattern */
		BEEMU_TOKENIZER_ARITHMATIC_INC_DEC,
		/** Emitted for direct arithmatic loads post mainline. */
		BEEMU_TOKENIZER_ARITHMATIC_DIRECT,
		/** Emitted for "weird" instructions of DAA, CPL, SCF and CCF*/
		BEEMU_TOKENIZER_ARITHMATIC_WEIRD,
		/** Emitted for 16 bit increment decrement operations */
		BEEMU_TOKENIZER_ARITHMATIC16_INC_DEC
	} BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE;

	/**
	 * @brief Given a partially initiated instruction tokenize it as a arithmatic
	 * instruction.
	 *
	 * @param instruction Partially constructed instruction known to be under
	 * arithmatic instruction space.
	 * @param opcode Opcode portion of the instruction for faster checks for some
	 * operations.
	 */
	void tokenize_arithmatic(BeemuInstruction* instruction, uint8_t opcode);

	/**
	 * @brief Check if the given OPCODE is an arithmatic operation and if so
	 * determine its subtype.
	 *
	 * @param opcode Opcode to check against, must not be a CBXX opcode.
	 * @return subtype of the arithmatic operation if one exists.
	 */
	BEEMU_TOKENIZER_ARITHMATIC_SUBTYPE
	arithmatic_subtype_if_arithmatic(uint8_t opcode);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_ARITHMATIC_H