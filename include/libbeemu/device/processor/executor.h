#ifndef BEEMU_INSTRUCTION_EXECUTOR_H
#define BEEMU_INSTRUCTION_EXECUTOR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../primitives/instruction.h"
#include "registers.h"
#include "../memory.h"

	/**
	 * @brief Execute a single instruction on the device memory and file.
	 *
	 * @param memory device memory.
	 * @param file register file.
	 * @param instruction Instruction to execute.
	 *
	 * @return elapsed clock cycles
	 */
	uint8_t execute_instruction(BeemuMemory *memory, BeemuRegisters *file, BeemuInstruction instruction);

#ifdef __cplusplus
}
#endif

#endif // BEEMU_INSTRUCTION_EXECUTOR_H