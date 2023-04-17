#ifndef BEEMU_INSTRUCTION_EXECUTOR_H
#define BEEMU_INSTRUCTION_EXECUTOR_H
#include "../primitives/instruction.h"
#include "registers.h"
#include "../memory.h"

/**
 * @brief Execute a single instruction on the device memory and file.
 *
 * @param memory device memory.
 * @param file register file.
 * @param instruction Instruction to execute.
 */
void execute_instruction(BeemuMemory *memory, BeemuRegisters *file, BeemuInstruction instruction);

#endif // BEEMU_INSTRUCTION_EXECUTOR_H