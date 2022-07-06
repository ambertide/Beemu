#include <device/device.h>
#include <device/memory.h>
#include <stdlib.h>

BeemuDevice *beemu_device_new(void)
{
	BeemuDevice *device = (BeemuDevice *)malloc(sizeof(BeemuDevice));
	device->memory = beemu_memory_new(BEEMU_DEVICE_MEMORY_SIZE);
	device->registers = beemu_registers_new();
	return device;
}

void beemu_device_free(BeemuDevice *device)
{
	beemu_memory_free(device->memory);
	beemu_registers_free(device->registers);
	free(device);
}

/**
 * @brief Peek the next instruction.
 *
 * Get the next instruction to be executed.
 * @param device
 */
static inline uint8_t peek_instruction(BeemuDevice *device)
{
	const uint16_t next_instruction_location = beemu_registers_read_16(device->registers, BEEMU_REGISTER_PC);
	const uint8_t next_instruction = beemu_memory_read(device->memory, next_instruction_location);
	return next_instruction;
}

/**
 * @brief Pop the next instruction.
 *
 * Get the next instruction to be executed, increment
 * the program counter.
 * @param device
 */
static inline uint16_t pop_instruction(BeemuDevice *device)
{
	const uint8_t next_instruction = peek_instruction(device);
	beemu_registers_increment_16(device->registers, BEEMU_REGISTER_PC);
	return next_instruction;
}

/**
 * @brief Decode register from instruction.
 *
 * Get the register for the LD and ADD instructions from the second nibble
 * of the instruction.
 *
 * @param instruction Instruction to decode.
 * @return const BeemuRegister_8 The decoded register.
 */
static inline const BeemuRegister_8 decode_register_from_instruction(uint8_t instruction)
{
	static const BeemuRegister_8 registers[8] = {BEEMU_REGISTER_B,
												 BEEMU_REGISTER_C,
												 BEEMU_REGISTER_D,
												 BEEMU_REGISTER_E,
												 BEEMU_REGISTER_H,
												 BEEMU_REGISTER_L,
												 BEEMU_REGISTER_A,
												 BEEMU_REGISTER_A};
	const uint8_t second_nibble = instruction & 0x0F;
	const int register_index = second_nibble >= 0x08 ? second_nibble - 0x08 : second_nibble;
	return registers[register_index];
}

/**
 * @brief Execute an arithmatic register instruction
 *
 * Execute ADD, ADC, SUB, SBC, AND, XOR, OR and CP with registers
 * affecting the acumulator.
 *
 * @param device BeemuDevice object.
 * @param instruction Instruction to execute.
 */
static inline void execute_arithmatic_register_instruction(BeemuDevice *device, uint8_t instruction)
{
	const uint8_t first_nibble = instruction & 0xF0;
	const uint8_t second_nibble = instruction & 0x0F;
	BeemuOperation operation = BEEMU_OP_ADD;
	switch (first_nibble)
	// Determine the operation.
	{
	case 0x80:
		operation = BEEMU_OP_ADD;
		break;
	case 0x90:
		operation = BEEMU_OP_SUB;
		break;
	case 0xA0:
		operation = BEEMU_OP_AND;
		if (second_nibble >= 0x08)
		{
			operation = BEEMU_OP_XOR;
		}
		break;
	case 0xB0:
		operation = BEEMU_OP_OR;
		if (second_nibble >= 0x08)
		{
			operation = BEEMU_OP_CP;
		}
		break;
	}
	const bool should_add_carry = second_nibble > 0x08 && first_nibble >= 0xA0;
	if (second_nibble == 0x06 || second_nibble == 0x0E)
	{
		// This uses the dereferenced HL register.
		const uint16_t memory_address = beemu_registers_read_16(device->registers, BEEMU_REGISTER_HL);
		const uint8_t dereferenced_value = beemu_memory_read(device->memory, memory_address);
		beemu_registers_arithmatic_8_constant(device->registers,
											  dereferenced_value,
											  operation,
											  should_add_carry);
	}
	else
	{
		beemu_registers_arithmatic_8_register(device->registers,
											  decode_register_from_instruction(instruction),
											  operation,
											  should_add_carry);
	}
}

void beemu_device_run(BeemuDevice *device)
{
	beemu_registers_write_16(device->registers, BEEMU_REGISTER_PC, BEEMU_DEVICE_MEMORY_ROM_LOCATION);
	while (true)
	{
		uint8_t next_instruction = pop_instruction(device);
		uint8_t first_nibble = next_instruction & 0xF0;
		uint8_t last_nibble = next_instruction & 0x0F;
		switch (first_nibble)
		{
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
			if (last_nibble != 0x07)
			{
				execute_arithmatic_register_instruction(device, next_instruction);
				break;
			}
			else
			{
			}
		}
	}
loop_stop:
	// Use a label here to avoid checking the conditions all the time.
	return;
}
