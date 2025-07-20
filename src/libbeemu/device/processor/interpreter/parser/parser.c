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
#include "parse_arithmatic.h"

BeemuCommandQueue *beemu_parser_parse(const BeemuProcessor *processor, const BeemuInstruction *instruction) {
	BeemuCommandQueue *queue = beemu_command_queue_new();
	switch (instruction->type) {
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC:
		parse_arithmatic(queue, processor, instruction);
		break;
	default:
		break;
	}
	return queue;
}
