/**
 * @file tokenize_load.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that contains LD and LDH instruction tokenization
 * logic that affect values being passed around.
 * @version 0.1
 * @date 2025-04-23
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <libbeemu/device/primitives/instruction.h>
#include <stdint.h>

#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_LOAD_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_LOAD_H
#ifdef __cplusplus
extern "C" {
#endif

	/**
	 * @brief Differentiate the Load8 subtype during tokenization stage.
	 *
	 */
	typedef enum BEEMU_TOKENIZER_LOAD_SUBTYPE {
		/** Emitted when not load. */
		BEEMU_TOKENIZER_LOAD8_INVALID_LOAD,
		/** Emitted when 0x40-0x7F mainline */
		BEEMU_TOKENIZER_LOAD8_MAINLINE,
		/** Emitted for 0xn6 - 0xnE premainline D8 instruction. */
		BEEMU_TOKENIZER_LOAD8_D8,
		/** Emitted for 0xn2 - 0xnA premainline M16 instruction */
		BEEMU_TOKENIZER_LOAD8_M16,
		/** Emitted for 0xEn and 0xFn postmainline ldh loads where n is either 0 or 2. */
		BEEMU_TOKENIZER_LOAD8_LDH,
		/** Emitted for 0xEA and 0xFA, postmainline ld addr16 loads. */
		BEEMU_TOKENIZER_LOAD8_ADDR16,
		/** Emitted for premainline immediate load operations */
		BEEMU_TOKENIZER_LOAD16_IMM16,
		/** Emitted for the push pop 16 postmainline instructions encoded as LD [SP] */
		BEEMU_TOKENIZER_LOAD16_PUSH_POP,
		/** Emitted for postmainline SP/HL transfer instruction */
		BEEMU_TOKENIZER_LOAD16_SP_HL
	} BEEMU_TOKENIZER_LOAD_SUBTYPE;

	/**
	 * @brief Given a partially initiated instruction tokenize it as a load instruction.
	 *
	 * @param instruction Partially constructed instruction known to be under LD or LDH
	 * instruction space.
	 * @param opcode Opcode portion of the instruction for faster checks for some operations.
	 */
	void tokenize_load(BeemuInstruction* instruction, uint8_t opcode);

	/**
	 * @brief Check if the given OPCODE is a load operation and if so determine its subtype.
	 *
	 * @param opcode Opcode to check against, must not be a CBXX opcode.
	 * @return subtype of the load operation if one exists.
	 */
	BEEMU_TOKENIZER_LOAD_SUBTYPE load_subtype_if_load(uint8_t opcode);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_LOAD_H