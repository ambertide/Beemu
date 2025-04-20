/**
 * @file tokenize_load8.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that includes tokenization functions for
 * 8bit load instructions, namely LD s operating on 8 bit registers, indirect
 * 16 bit registers and LDH instructions.
 * @version 0.1
 * @date 2025-04-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <libbeemu/device/primitives/instruction.h>
#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_CBXX_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_CBXX_H
#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * @brief Tokenize a CB prefixed instruction.
	 *
	 * This tokenizes the 0xCB00-0xCBFF block of instructions.
	 *
	 * @param instruction Partially created instruction to tokenize.
	 */
	void cb_tokenize(BeemuInstruction *instruction);

#ifdef __cplusplus
}
#endif
#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_CBXX_H