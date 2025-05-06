#ifndef BEEMU_TOKENIZER_H
#define BEEMU_TOKENIZER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "../primitives/instruction.h"

	BeemuInstruction tokenize_instruction(uint8_t instruction);

	/**
	 * @brief Tokenize an instruction.
	 *
	 * @param instruction Instruction to tokenize, instructions are considered
	 * right padded, with the leftmost byte always being 0x00 and the latter
	 * three bytes possibly being important.
	 * @return BeemuInstruction Instrcution, tokenized.
	 */
	BeemuInstruction *beemu_tokenizer_tokenize(uint32_t instruction);

	/**
	 * @brief Free a instruction token.
	 *
	 * @param token token to free.
	 */
	void beemu_tokenizer_free_token(BeemuInstruction *token);

#ifdef __cplusplus
}
#endif
#endif // BEEMU_TOKENIZER_H