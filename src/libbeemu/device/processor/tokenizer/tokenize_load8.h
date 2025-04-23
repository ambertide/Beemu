/**
 * @file tokenize_load8.h
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
extern "C"
{
#endif
	/**
	 * @brief Given a partially initiated instruction tokenize it as a load instruction.
	 *
	 * @param instruction Partially constructed instruction known to be under LD or LDH
	 * instruction space.
	 * @param opcode Opcode portion of the instruction for faster checks for some operations.
	 */
	void tokenize_load8(BeemuInstruction *instruction, uint8_t opcode);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_LOAD_H