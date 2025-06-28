/**
 * @file tokenize_jump.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that holds tokenization logic for
 * RST, RET, CALL, etc.
 * @version 0.1
 * @date 2025-06-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_JUMP_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_JUMP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <libbeemu/device/primitives/instruction.h>
#include <stdint.h>

	/**
	 * @brief Differentiate the jump subtype during tokenization stage.
	 *
	 */
	typedef enum BEEMU_TOKENIZER_JUMP_SUBTYPE {
		/** Emitted when not jump. */
		BEEMU_TOKENIZER_JUMP_INVALID_JUMP,
		/** Emitted for unconditional relative jump */
		BEEMU_TOKENIZER_JUMP_RELATIVE_UNCONDITIONAL,
		/** Emitted for conditional relative jump */
		BEEMU_TOKENIZER_JUMP_RELATIVE_CONDITIONAL,
		/** Emitted for unconditional jumps. */
		BEEMU_TOKENIZER_JUMP_UNCONDITIONAL,
		/** Emitted for conditional jumps */
		BEEMU_TOKENIZER_JUMP_CONDITIONAL,
		/** Emitted for RST instructions */
		BEEMU_TOKENIZER_JUMP_RST
	} BEEMU_TOKENIZER_JUMP_SUBTYPE;

	/**
	 * @brief Given a partially initiated instruction tokenize it as a jump
	 * instruction.
	 *
	 * @param instruction Partially constructed instruction known to be under
	 * jump instruction space.
	 * @param opcode Opcode portion of the instruction for faster checks for some
	 * operations.
	 */
	void tokenize_jump(BeemuInstruction* instruction, uint8_t opcode);

	/**
	 * @brief Check if the given OPCODE is an jump operation and if so
	 * determine its subtype.
	 *
	 * @param opcode Opcode to check against, must not be a CBXX opcode.
	 * @return subtype of the jump operation if one exists.
	 */
	BEEMU_TOKENIZER_JUMP_SUBTYPE
	jump_subtype_if_jump(uint8_t opcode);

#ifdef __cplusplus
}
#endif
#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_JUMP_H
