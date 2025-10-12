#include "../../src/beemu/device/processor/interpreter/command.h"
#include <BeemuTest.hpp>
#include <gtest/gtest.h>
#include <beemu/device/primitives/instruction.h>
#include <stdbool.h>

namespace BeemuTests {
TEST_F(BeemuTestFixture, CommandQueueNew)
{
	const BeemuCommandQueue *queue = beemu_command_queue_new();
	EXPECT_EQ(queue->first, nullptr);
	EXPECT_EQ(queue->last, nullptr);
}

TEST_F(BeemuTestFixture, CommandQueueEnqueue)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;
	beemu_command_queue_enqueue(queue, &command);
	// Values must be the same.
	EXPECT_EQ(command.type, queue->first->current->type);
	EXPECT_EQ(command.halt.halt_operation, queue->first->current->halt.halt_operation);
	EXPECT_EQ(command.halt.is_cycle_terminator, queue->first->current->halt.is_cycle_terminator);
}

TEST_F(BeemuTestFixture, CommandQueueEnqueueShouldCopy)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;
	beemu_command_queue_enqueue(queue, &command);
	// The addresses must be different.
	EXPECT_NE(&command, queue->first->current);
}

TEST_F(BeemuTestFixture, CommandQueueEnqueueFirstLastTrackingShouldBeCorrect)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();

	// Enqueue three commands
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;

	BeemuMachineCommand second_command;
	second_command.type = BEEMU_COMMAND_WRITE;
	second_command.write.target = {BEEMU_WRITE_TARGET_REGISTER_8, { .register_8=BEEMU_REGISTER_A}};
	second_command.write.value = {false, {.byte_value=0xFF}};

	beemu_command_queue_enqueue(queue, &command);
	// Enqueue this twice
	beemu_command_queue_enqueue(queue, &command);
	beemu_command_queue_enqueue(queue, &second_command);

	// Expect first and last to be the correct ones.

	EXPECT_EQ(command.type, queue->first->current->type);
	EXPECT_EQ(second_command.type, queue->last->current->type);
	// Also check if internal nodes are correct.
	EXPECT_EQ(queue->first->next->next->current->type, second_command.type);
}

TEST_F(BeemuTestFixture, CommandQueueDequeueShouldReturnFirstMember)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();

	// Enqueue three commands
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;

	BeemuMachineCommand second_command;
	second_command.type = BEEMU_COMMAND_WRITE;
	second_command.write.target = {BEEMU_WRITE_TARGET_REGISTER_8, { .register_8=BEEMU_REGISTER_A}};
	second_command.write.value = {false, {.byte_value=0xFF}};

	beemu_command_queue_enqueue(queue, &command);
	beemu_command_queue_enqueue(queue, &second_command);


	// After we dequeue
	BeemuMachineCommand *returned_command = beemu_command_queue_dequeue(queue);

	// Returned to be correct.
	EXPECT_EQ(returned_command->type, command.type);
	EXPECT_EQ(returned_command->write.value.value.byte_value, command.write.value.value.byte_value);
}

TEST_F(BeemuTestFixture, CommandQueueDequeueShouldCorrectlyReturnToEmptyState)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();

	// Enqueue three commands
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;

	BeemuMachineCommand second_command;
	second_command.type = BEEMU_COMMAND_WRITE;
	second_command.write.target = {BEEMU_WRITE_TARGET_REGISTER_8, { .register_8=BEEMU_REGISTER_A}};
	second_command.write.value = {false, {.byte_value=0xFF}};

	beemu_command_queue_enqueue(queue, &command);
	beemu_command_queue_enqueue(queue, &second_command);


	// After we dequeue twice we should return to empty state.
	beemu_command_queue_dequeue(queue);
	beemu_command_queue_dequeue(queue);

	// Expect first and last to be the correct ones.
	EXPECT_EQ(queue->first, nullptr);
	EXPECT_EQ(queue->last, nullptr);
}

TEST_F(BeemuTestFixture, CommandQueueDequeueShouldUpdateFirstPointer)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();

	// Enqueue three commands
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;

	BeemuMachineCommand second_command;
	second_command.type = BEEMU_COMMAND_WRITE;
	second_command.write.target = {BEEMU_WRITE_TARGET_REGISTER_8, { .register_8=BEEMU_REGISTER_A}};
	second_command.write.value = {false, {.byte_value=0xFF}};

	beemu_command_queue_enqueue(queue, &command);
	beemu_command_queue_enqueue(queue, &second_command);
	beemu_command_queue_enqueue(queue, &command);

	// After we dequeue
	beemu_command_queue_dequeue(queue);

	// Expect first and last to be the correct ones.
	EXPECT_EQ(queue->last->current->type, command.type);
	EXPECT_EQ(queue->first->current->type, second_command.type);
	EXPECT_EQ(queue->first->next, queue->last);
}

TEST_F(BeemuTestFixture, CommandQueueIsEmptyShouldBeTrueOnNew)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();
	EXPECT_TRUE(beemu_command_queue_is_empty(queue));
}

TEST_F(BeemuTestFixture, CommandQueueIsEmptyShouldBeFalseAfterEnqueue)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();
	// Enqueue first.
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;
	beemu_command_queue_enqueue(queue, &command);
	EXPECT_FALSE(beemu_command_queue_is_empty(queue));
}

TEST_F(BeemuTestFixture, CommandQueueEmptyAfterDequeue)
{
	BeemuCommandQueue *queue = beemu_command_queue_new();
	// Enqueue first.
	BeemuMachineCommand command;
	command.type = BEEMU_COMMAND_HALT;
	command.halt.halt_operation = BEEMU_CPU_OP_HALT;
	command.halt.is_cycle_terminator = false;
	beemu_command_queue_enqueue(queue, &command);
	// Now dequeue
	beemu_command_queue_dequeue(queue);
	EXPECT_TRUE(beemu_command_queue_is_empty(queue));
}
}
