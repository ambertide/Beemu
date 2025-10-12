/**
 * @file parse_jump.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-10-12
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PARSE_JUMP_H
#define BEEMU_PARSE_JUMP_H
#include "libbeemu/device/processor/processor.h"
#include "parse_common.h"

/**
 * Parse a jump instruction to write commands.
 * @param queue CommandQueue to append the write commands to
 * @param processor Processor context
 * @param instruction Instruction to parse.
 */
void parse_jump(
	BeemuCommandQueue *queue,
	const BeemuProcessor *processor,
	const BeemuInstruction *instruction);



#endif // BEEMU_PARSE_JUMP_H
