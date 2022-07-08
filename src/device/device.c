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
	device->current_instruction.instruction = next_instruction;
	device->current_instruction.first_nibble = next_instruction & 0xF0;
	device->current_instruction.second_nibble = next_instruction & 0x0F;
	beemu_registers_increment_16(device->registers, BEEMU_REGISTER_PC);
	return next_instruction;
}

/**
 * @brief Pop and load data to device from memory.
 *
 * @param device BeemuDevice object pointer.
 */
static inline void pop_data(BeemuDevice *device, bool is_byte_length)
{
	if (is_byte_length)
	{
		const uint8_t data = peek_instruction(device);
		// Increment the PC register.
		beemu_registers_increment_16(device->registers, BEEMU_REGISTER_PC);
		device->data.data_8 = data;
	}
	else
	{
		// For 16 bit data.
		const uint8_t high = peek_instruction(device);
		beemu_registers_increment_16(device->registers, BEEMU_REGISTER_PC);
		const uint8_t lower = peek_instruction(device);
		beemu_registers_increment_16(device->registers, BEEMU_REGISTER_PC);
		device->data.data_16 = (((uint16_t)high) << 8) | ((uint16_t)lower);
	}
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
 * @brief Write to the memory location at the (HL)
 *
 * Dereference HL and write to the memory location it points to.
 * @param device BeemuDevice object pointer.
 * @param value Value to write.
 */
void write_to_dereferenced_hl(BeemuDevice *device, uint8_t value)
{
	const uint16_t memory_address = beemu_registers_read_16(device->registers, BEEMU_REGISTER_HL);
	beemu_memory_write(device->memory, memory_address, value);
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
 */
static inline void execute_arithmatic_register_instruction(BeemuDevice *device)
{
	BeemuOperation operation = BEEMU_OP_ADD;
	switch (device->current_instruction.first_nibble)
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
		if (device->current_instruction.second_nibble >= 0x08)
		{
			operation = BEEMU_OP_XOR;
		}
		break;
	case 0xB0:
		operation = BEEMU_OP_OR;
		if (device->current_instruction.second_nibble >= 0x08)
		{
			operation = BEEMU_OP_CP;
		}
		break;
	}
	const bool should_add_carry = device->current_instruction.second_nibble > 0x08 && device->current_instruction.first_nibble >= 0xA0;
	if (device->current_instruction.second_nibble == 0x06 || device->current_instruction.second_nibble == 0x0E)
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
											  decode_register_from_instruction(device->current_instruction.instruction),
											  operation,
											  should_add_carry);
	}
}

/**
 * @brief Execute a load instruction.
 *
 * @param device BeemuDevice object.
 */
static inline void execute_load_instruction(BeemuDevice *device)
{
	const BeemuRegister_8 destination_register = decode_register_from_instruction(device->current_instruction.instruction);
	uint8_t value = dereference_hl(device);
	if (!(device->current_instruction.first_nibble == 0x70 && device->current_instruction.second_nibble <= 0x07))
	{
		// If outside this block, then read from a register.
		const BeemuRegister_8 source_register = decode_ld_register_from_instruction(device->current_instruction.instruction);
		value = beemu_registers_read_8(device->registers, source_register);
	}
	if (device->current_instruction.second_nibble != 0x06 && device->current_instruction.second_nibble != 0x0E)
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

/**
 * @brief Execute unary inc or dec on memory.
 *
 * Execute an increment or a decrement operation on the
 * value of a memory address dereferenced by HL and
 * write it to the same memory, set the flags.
 * @param device BeemuDevice object pointer.
 * @param operation Operation to perform.
 */
void execute_unary_on_memory(BeemuDevice *device, BeemuUnaryOperation operation)
{
	uint8_t previous_value = dereference_hl(device);
	const uint8_t new_value = operation == BEEMU_UOP_INC ? previous_value + 1 : previous_value - 1;
	// Binary equivalent of the unary operation, INC => ADD, DEC => SUB.
	const uint8_t op_equivalent = operation == BEEMU_UOP_INC ? BEEMU_OP_ADD : BEEMU_OP_SUB;
	write_to_dereferenced_hl(device, new_value);
	beemu_registers_set_flags(device->registers, previous_value, new_value, false, op_equivalent, true);
}

/**
 * @brief Decode 03 Block's symmetric register encodings.
 *
 * Some instructions like the LD direct, INC and DEC instructions of
 * the 03 block has symmetrical encoding of registers depending on whether
 * or not the second nibble is lower than 0x07.
 * @param device BeemuDevice object pointer.
 * @return BeemuRegister_8 the decoded register.
 */
BeemuRegister_8 decode_03_block_sym_register(BeemuDevice *device)
{
	const bool on_the_left = device->current_instruction.second_nibble < 0x07;
	const int row = on_the_left ? 0 : 1;
	const int column = device->current_instruction.first_nibble >> 1;
	return SYMMETRIC_REGISTERS[column][row];
}

/**
 * @brief Execute a register increment or decrement operation.
 *
 * Execute a register increment or decrement operation, the register to
 * operate on depends on the instruction.
 * @param device BeemuDevice object pointer.
 * @param increment If set to true, increment, otherwise decrement.
 */
void execute_unary_operand(BeemuDevice *device, bool increment)
{
	const bool on_the_left = device->current_instruction.second_nibble < 0x07;
	const BeemuUnaryOperation uop = increment ? BEEMU_UOP_INC : BEEMU_UOP_DEC;
	if (on_the_left && device->current_instruction.first_nibble == 0x30)
	{
		// These act on the dereferenced HL.
		execute_unary_on_memory(device, uop);
	}
	else
	{
		beemu_registers_airthmatic_8_unary(device->registers,
										   decode_03_block_sym_register(device),
										   uop);
	}
}

/**
 * @brief Execute a register increment or decrement operation on 16.
 *
 * Execute a register increment or decrement operation, the register to
 * operate on depends on the instruction.
 * @param device BeemuDevice object pointer.
 */
void execute_unary_operand_16(BeemuDevice *device)
{
	const bool on_the_left = device->current_instruction.second_nibble < 0x07;
	const int index = device->current_instruction.first_nibble >> 1;
	const BeemuUnaryOperation uop = on_the_left ? BEEMU_UOP_INC : BEEMU_UOP_DEC;
	beemu_registers_airthmatic_16_unary(device->registers, ORDERED_REGISTER_NAMES_16[index], uop);
}

/**
 * @brief Execute an arithmatic register instruction
 *
 * Execute ADD with 16 bitregisters
 * affecting the HL.
 *
 * @param device BeemuDevice object.
 */
static inline void execute_arithmatic_register_instruction_16(BeemuDevice *device)
{
	// Only supported 16 bit OP is ADD (for some reason.).
	BeemuOperation operation = BEEMU_OP_ADD;
	const int index = device->current_instruction.first_nibble >> 1;
	beemu_registers_arithmatic_16_register(device->registers,
										   ORDERED_REGISTER_NAMES_16[index],
										   operation);
}

/**
 * @brief Load directly to a register
 *
 * @param device BeemuDevice object pointer.
 * @param is_byte_length If set to true, interpret as a load to
 * a 16-byte register.
 */
void execute_load_direct(BeemuDevice *device, bool is_byte_length)
{
	if (is_byte_length)
	{
		// Load 8 bit data.
		pop_data(device, true);
		const BeemuRegister_8 destination_register = decode_03_block_sym_register(device);
		beemu_registers_write_8(device->registers, destination_register, device->data.data_8);
	}
	else
	{
		// Load 16 bit data
		pop_data(device, false);
		const BeemuRegister_16 destination_register = ORDERED_REGISTER_NAMES_16[device->current_instruction.first_nibble >> 1];
		beemu_registers_write_16(device->registers, destination_register, device->data.data_16);
	}
}

/**
 * @brief Execute the block of instructions between row 0x00 and 0x30.
 *
 * These blocks of instructions display a periodic table like behaviour
 * depending on the last nibble.
 * @param device BeemuDevice object pointer.
 */
void execute_block_03(BeemuDevice *device)
{
	switch (device->current_instruction.second_nibble)
	{
	case 0x00:
		switch (device->current_instruction.first_nibble)
		{
		case 0x00:
			break;
		case 0x01:
			beemu_device_set_state(device, BEEMU_DEVICE_STOP);
			break;
		default:
			break;
		}
	case 0x01:
		execute_load_direct(device, false);
	case 0x04:
	case 0x05:
	case 0x0C:
	case 0x0D:
		// INC and DEC
		execute_unary_operand(device, device->current_instruction.second_nibble == 0x04 || device->current_instruction.second_nibble == 0x0C);
		break;
	case 0x06:
	case 0x0E:
		execute_load_direct(device, true);
		break;
	case 0x09:
		execute_arithmatic_register_instruction_16(device);
		break;
	default:
		break;
	}
}

void beemu_device_run(BeemuDevice *device)
{
	beemu_registers_write_16(device->registers, BEEMU_REGISTER_PC, BEEMU_DEVICE_MEMORY_ROM_LOCATION);
	while (true)
	{
		uint8_t next_instruction = pop_instruction(device);
		switch (device->current_instruction.first_nibble)
		{
		case 0x00:
		case 0x10:
		case 0x20:
		case 0x30:
			execute_block_03(device);
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
			if (device->current_instruction.second_nibble == 0x06)
			{
				// HALT
				beemu_device_set_state(device, BEEMU_DEVICE_HALT);
			}
			else
			{
				execute_load_instruction(device);
			}
			break;
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
			execute_arithmatic_register_instruction(device);
			break;
		}
	}
loop_stop:
	// Use a label here to avoid checking the conditions all the time.
	return;
}
