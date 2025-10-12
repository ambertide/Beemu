/**
 * @file command.h
 * @author Ege Özkan (elsaambertide@gmail.com)
 * @brief Private header file that contains BeemuMachineCommand
 * @version 0.1
 * @date 2025-06-29
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BEEMU_PROCESSOR_PARSER_COMMAND_H
#define BEEMU_PROCESSOR_PARSER_COMMAND_H
#ifdef __cplusplus
extern "C" {
#endif
#include "beemu/device/processor/registers.h"
#include <beemu/device/primitives/instruction.h>
#include <stdint.h>
#include <stdbool.h>

	typedef struct BeemuHaltCommand {
		/** true for special commands that terminate cycles, false for normal halt instructions */
		bool is_cycle_terminator;
		/** Meaningful for system halt instructions. */
		BeemuSystemOperation halt_operation;
	} BeemuHaltCommand;

	typedef enum BeemuWriteTargetType {
		BEEMU_WRITE_TARGET_REGISTER_16,
		BEEMU_WRITE_TARGET_REGISTER_8,
		BEEMU_WRITE_TARGET_MEMORY_ADDRESS,
		BEEMU_WRITE_TARGET_FLAG,
		BEEMU_WRITE_TARGET_IME,
		// Reserved for internal gameboy features we do not use
		// but want to emulate because why not.
		BEEMU_WRITE_TARGET_INTERNAL
	} BeemuWriteTargetType;

	typedef enum BeemuInternalTargetType {
		BEEMU_INTERNAL_WRITE_TARGET_ADDRESS_BUS,
		BEEMU_INTERNAL_WRITE_TARGET_DATA_BUS,
		BEEMU_INTERNAL_WRITE_TARGET_PROGRAM_COUNTER,
		BEEMU_INTERNAL_WRITE_TARGET_INSTRUCTION_REGISTER
	} BeemuInternalTargetType;

	/**
	 * Used to desribe where to write in the machine state.
	 */
	typedef struct BeemuWriteTarget {
		BeemuWriteTargetType type;
		union {
			BeemuRegister_16 register_16;
			BeemuRegister_8 register_8;
			uint16_t mem_addr;
			BeemuFlag flag;
			BeemuInternalTargetType internal_target;
		} target;
	} BeemuWriteTarget;

	/**
	 * Used to describe what to write into machine state.
	 */
	typedef struct BeemuWriteValue {
		/** true if the value is 16 bit. */
		bool is_16;
		union {
			uint8_t byte_value;
			uint16_t double_value;
		} value;
	} BeemuWriteValue;

	/**
	 * A command that when executed changes a value in the machine state.
	 */
	typedef struct BeemuWriteCommand {
		BeemuWriteTarget target;
		BeemuWriteValue value;
	} BeemuWriteCommand;

	typedef enum BeemuCommandType {
		BEEMU_COMMAND_HALT,
		BEEMU_COMMAND_WRITE
	} BeemuCommandType;

	/**
	 * Represents a machine command that must be executed
	 * by the invoker.
	 */
	typedef struct BeemuMachineCommand {
		BeemuCommandType type;
		union {
			BeemuWriteCommand write;
			BeemuHaltCommand halt;
		};
	} BeemuMachineCommand;


	struct BeemuCommandQueueNode;

	/**
	 * Used internally in a command queue, holds a single command.
	 */
	typedef struct BeemuCommandQueueNode {
		BeemuMachineCommand* current;
		struct BeemuCommandQueueNode* next;
	} BeemuCommandQueueNode;

	/**
	 * Holds a ordered stream of commands.
	 */
	typedef struct BeemuCommandQueue {
		BeemuCommandQueueNode *first;
		BeemuCommandQueueNode *last;
	} BeemuCommandQueue;

	/**
	 * Create a new command queue.
	 * @return The newly generated command queue.
	 */
	BeemuCommandQueue *beemu_command_queue_new();

	/**
	 * Free an existing command queue.
	 * @param queue Queue to free.
	 */
	void beemu_command_queue_free(BeemuCommandQueue *queue);

	/**
	 * Dequeue the next command from the command queue.
	 * @param queue Command queue to act on.
	 * @return The next command on the queue.
	 */
	BeemuMachineCommand *beemu_command_queue_dequeue(BeemuCommandQueue *queue);

	/**
	 * Peek to the next command in the queue.
	 * @param queue Queue to act on.
	 * @return Const ptr to the next command in the queue.
	 */
	const BeemuMachineCommand *beemu_command_queue_peek(BeemuCommandQueue *queue);

	/**
	 * Enqueue a command to the end of the queue.
	 * @param queue Queue to act on.
	 * @param command Command to enqueue.
	 */
	void beemu_command_queue_enqueue(BeemuCommandQueue *queue, const BeemuMachineCommand *command);

	/**
	 * Check whether the queue is empty.
	 * @param queue Queue to check.
	 * @return true if the queue is empty, false otherwise.
	 */
	bool beemu_command_queue_is_empty(BeemuCommandQueue *queue);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_PROCESSOR_PARSER_COMMAND_H