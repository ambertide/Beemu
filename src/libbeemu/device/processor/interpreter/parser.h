/**
 * @file parser.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief BeemuP
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PARSER_H
#define BEEMU_PARSER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "command.h"
#include "libbeemu/device/processor/processor.h"

/**
 * @brief Parse an instruction to a series of commands.
 *
 * Parse a given BeemuInstruction given the current state of the processor,
 * return a list of commands that if invoked by the BeemuInvoker, modifies
 * the processor state to the one intended by the Instruction.
 * @param processor BeemuProcessor to act on.
 * @param instruction Instruction to parse.
 * @return A series of commands that can be invoked to modify the processor state
 * to the intended result.
 */
BeemuCommandQueue *beemu_parser_parse(const BeemuProcessor *processor, const BeemuInstruction *instruction);

#endif // BEEMU_PARSER_H
#ifdef __cplusplus
	}
#endif
