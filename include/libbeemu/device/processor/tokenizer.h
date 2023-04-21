#ifndef BEEMU_TOKENIZER_H
#define BEEMU_TOKENIZER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../primitives/instruction.h"

	BeemuInstruction tokenize_instruction(uint8_t instruction);

#ifdef __cplusplus
}
#endif
#endif // BEEMU_TOKENIZER_H