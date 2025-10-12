/**
* @file tokenize_system.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that contains system instructions
 * @version 0.1
 * @date 2025-06-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_SYSTEM_H
#define BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_SYSTEM_H
#include "beemu/device/primitives/instruction.h"

/**
 * Tokenize system instructions that control the CPU state
 * and special flags like interrupt.
 *
 * As a side effect tokenize and mutate the *instruction.
 *
 * @param instruction Partially decoded instruction.
 * @param opcode Canonical single-byte opcode.
 * @return true if system instruction, false otherwise.
 */
bool tokenize_system(BeemuInstruction* instruction, uint8_t opcode);

#endif // BEEMU_PROCESSOR_TOKENIZER_TOKENIZE_SYSTEM_H
