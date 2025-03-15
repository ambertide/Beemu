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
	 * @param instruction Instruction to tokenize
	 * @return BeemuInstruction Instrcution, tokenized.
	 */
	BeemuInstruction *beemu_tokenizer_tokenize(uint16_t instruction);

#ifdef __cplusplus
}
#endif
#endif // BEEMU_TOKENIZER_H