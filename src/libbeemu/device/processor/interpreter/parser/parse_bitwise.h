/**
 * @file parse_bitwise.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-08-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PARSE_BITWISE_H
#define BEEMU_PARSE_BITWISE_H
#include "parse_common.h"
#include <libbeemu/device/processor/processor.h>

/**
 * @brief Parse a bitwise token from CBXX range.
 *
 * Given a queue, the current state of the processor and a bitwise instruction token,
 * parse the token and populate the queue with the resulting write and halt command.
 * @param queue Queue to populate
 * @param processor Current processor state.
 * @param instruction Instruction to parse.
 */
void parse_bitwise(BeemuCommandQueue *queue, const BeemuProcessor *processor, const BeemuInstruction *instruction);

#endif // BEEMU_PARSE_BITWISE_H
