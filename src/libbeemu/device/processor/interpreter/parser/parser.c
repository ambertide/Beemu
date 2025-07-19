/**
 * @file parser.c
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Instructions
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "parser.h"

BeemuCommandQueue *beemu_parser_parse(const BeemuProcessor *processor, const BeemuInstruction *instruction) {
	BeemuCommandQueue *queue = beemu_command_queue_new();
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.is_cycle_terminator = true;
	beemu_command_queue_enqueue(queue, &command);
	return queue;
}
