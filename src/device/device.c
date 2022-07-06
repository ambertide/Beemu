#include <device/device.h>
#include <device/memory.h>
#include <stdlib.h>

BeemuDevice *beemu_device_new(void)
{
	BeemuDevice *device = (BeemuDevice *)malloc(sizeof(BeemuDevice));
	device->memory = beemu_memory_new(BEEMU_DEVICE_MEMORY_SIZE);
	device->registers = beemu_registers_new();
	device->interrupts_enabled = true;
	device->device_state = BEEMU_DEVICE_NORMAL;
	return device;
}

void beemu_device_free(BeemuDevice *device)
{
	beemu_memory_free(device->memory);
	beemu_registers_free(device->registers);
	free(device);
}

BeemuDeviceState beemu_device_get_device_state(BeemuDevice *device)
{
	return device->device_state;
}

void beemu_device_set_state(BeemuDevice *device, BeemuDeviceState state)
{
	device->device_state = state;
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
 * @brief Dereference the HL register.
 *
 * Get the value of the HL register and then return the value
 * that is inside the memory address at that value.
 * @param device BeemuDevice object.
 * @return uint8_t the dereferenced HL register.
 */
uint8_t dereference_hl(BeemuDevice *device)
{
	const uint16_t memory_address = beemu_registers_read_16(device->registers, BEEMU_REGISTER_HL);
	const uint8_t dereferenced_value = beemu_memory_read(device->memory, memory_address);
	return dereferenced_value;
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
	const uint8_t second_nibble = instruction & 0x0F;
	const int register_index = second_nibble >= 0x08 ? second_nibble - 0x08 : second_nibble;
	return ORDERED_REGISTER_NAMES[register_index];
}

/**
 * @brief Decode the source register for LD instruction.
 *
 * LD instructions of the main block (the larger LD block) has the
 * source register that can be interpreted from the instruction // 8.
 *
 * @param instruction Instruction to parse.
 * @return const BeemuRegister_8 Source register name.
 */
static inline const BeemuRegister_8 decode_ld_register_from_instruction(uint8_t instruction)
{
	const uint8_t register_index = (instruction - 0x40) / 8;
	return ORDERED_REGISTER_NAMES[register_index];
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
		const uint8_t dereferenced_value = dereference_hl(device);
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

/**
 * @brief Execute a load instruction.
 *
 * @param device BeemuDevice object.
 * @param instruction Instruction to execute.
 */
static inline void execute_load_instruction(BeemuDevice *device, uint8_t instruction)
{
	const uint8_t first_nibble = instruction & 0xF0;
	const uint8_t second_nibble = instruction & 0x0F;
	const BeemuRegister_8 destination_register = decode_register_from_instruction(instruction);
	uint8_t value = dereference_hl(device);
	if (!(first_nibble == 0x70 && second_nibble <= 0x07))
	{
		// If outside this block, then read from a register.
		const BeemuRegister_8 source_register = decode_ld_register_from_instruction(instruction);
		value = beemu_registers_read_8(device->registers, source_register);
	}
	if (second_nibble != 0x06 && second_nibble != 0x0E)
	{
		// Load to the dereferenced (HL) memory location.
		beemu_memory_write(device->memory, dereference_hl(device), value);
	}
	else
	{
		// Otherwise laod to the destination register.
		beemu_registers_write_8(device->registers, destination_register, value);
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
			if (last_nibble == 0x06)
			{
				// HALT
				beemu_device_set_state(device, BEEMU_DEVICE_HALT);
			}
			else
			{
				execute_load_instruction(device, next_instruction);
			}
			break;
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
			execute_arithmatic_register_instruction(device, next_instruction);
			break;
		}
	}
loop_stop:
	// Use a label here to avoid checking the conditions all the time.
	return;
}
