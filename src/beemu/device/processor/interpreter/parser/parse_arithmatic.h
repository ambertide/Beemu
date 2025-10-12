/**
 * @file parse_arithmatic.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Parse instructions related to `BEEMU_INSTRUCTION_TYPE_ARITHMATIC`
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PARSE_ARITHMATIC_H
#define BEEMU_PARSE_ARITHMATIC_H
#include "parse_common.h"
#include <beemu/device/processor/processor.h>

/**
 * @brief Parse an arithmatic token.
 *
 * Given a queue, the current state of the processor and an arithmatic instruction token,
 * parse the token and populate the queue with the resulting tokens.
 * @param queue Queue to populate
 * @param processor Current processor state.
 * @param instruction Instruction to parse.
 */
void parse_arithmatic(BeemuCommandQueue *queue, const BeemuProcessor *processor, const BeemuInstruction *instruction);

#endif // BEEMU_PARSE_ARITHMATIC_H
