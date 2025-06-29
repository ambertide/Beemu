//
// Created by Elsa on 29.06.2025.
//

#include "command.h"

#include <string.h>
#include <stdlib.h>

BeemuCommandQueue *beemu_command_queue_new()
{
	BeemuCommandQueue *queue = malloc(sizeof(BeemuCommandQueue));
	queue->first = 0;
	queue->last = 0;
	return queue;
};

void beemu_command_queue_free(BeemuCommandQueue *queue)
{
	queue->first = 0;
	queue->last = 0;
	free(queue);
};

bool beemu_command_queue_is_empty(BeemuCommandQueue *queue)
{
	return queue->first == 0;
}


void beemu_command_queue_enqueue(BeemuCommandQueue *queue, const BeemuMachineCommand *command)
{
	// Command is copied to a new memory address.
	BeemuMachineCommand *command_cpy = malloc(sizeof(BeemuMachineCommand));
	BeemuCommandQueueNode *node = malloc(sizeof(BeemuCommandQueueNode));
	// Set to nullptr so we can keep track of the end.
	node->next = 0;
	node->current = command_cpy;
	memcpy(command_cpy, command, sizeof(BeemuMachineCommand));
	if (beemu_command_queue_is_empty(queue)) {
		// If the queue is empty, then this is the first insert.
		queue->first = node;
		queue->last = queue->first;
	} else {
		// Otherwise, we need to add a new node to the last node.
		queue->last->next = node;
		// And update the last ptr.
		queue->last = node;
	}
};

BeemuMachineCommand *beemu_command_queue_dequeue(BeemuCommandQueue *queue)
{
	if (queue->first == 0) {
		return 0;
	}
	BeemuCommandQueueNode *first_node = queue->first;
	BeemuMachineCommand *first_command = first_node->current;
	// Increment the ptr.
	queue->first = queue->first->next;
	// Free the first node so we won't leak memory.
	free(first_node);
	if (beemu_command_queue_is_empty(queue)) {
		queue->last = 0;
	}
	return first_command;
};

const BeemuMachineCommand *beemu_command_queue_peek(BeemuCommandQueue *queue)
{
	if (queue->first == 0) {
		return 0;
	}
	BeemuMachineCommand *first_command = queue->first->current;
	return first_command;
};


