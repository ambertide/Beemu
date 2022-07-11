#include <stdlib.h>
#include <device/device.h>
#include <device/memory.h>
#include <internals/utility.h>

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
	case 0xC0:
		operation = BEEMU_OP_ADD;
		break;
	case 0x90:
	case 0xD0:
		operation = BEEMU_OP_SUB;
		break;
	case 0xA0:
	case 0xE0:
		operation = BEEMU_OP_AND;
		if (device->current_instruction.second_nibble >= 0x08)
		{
			operation = BEEMU_OP_XOR;
		}
		break;
	case 0xB0:
	case 0xF0:
		operation = BEEMU_OP_OR;
		if (device->current_instruction.second_nibble >= 0x08)
		{
			operation = BEEMU_OP_CP;
		}
		break;
	}
	const bool should_add_carry = device->current_instruction.second_nibble > 0x08 && device->current_instruction.first_nibble >= 0xA0;
	if (device->current_instruction.first_nibble >= 0xC0)
	{
		pop_data(device, true);
		beemu_registers_arithmatic_8_constant(device->registers,
											  device->data.data_8,
											  operation,
											  device->current_instruction.instruction == 0xCE && device->current_instruction.instruction == 0xDE);
	}
	else if (device->current_instruction.second_nibble == 0x06 || device->current_instruction.second_nibble == 0x0E)
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
		beemu_registers_arithmatic_8_unary(device->registers,
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
	beemu_registers_arithmatic_16_unary(device->registers, ORDERED_REGISTER_NAMES_16[index], uop);
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
 * a 8 register.
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
 * @brief Execute LD to/from dereferenced memory addresses to/from accumulator.
 *
 * @param device BeemuDevice object pointer.
 * @param from_accum if set to true, load from accumulator to the
 * dereferenced memory location, otherwise, load from the dereferenced
 * memory location to the accumulator register.
 */
static void execute_load_accumulator_16(BeemuDevice *device, bool from_accum)
{
	// Finally, you may have to decrement or increment.
	static const BeemuRegister_16 registers[4] = {
		BEEMU_REGISTER_BC,
		BEEMU_REGISTER_DE,
		BEEMU_REGISTER_HL,
		BEEMU_REGISTER_HL};
	const BeemuRegister_16 register_ = registers[register_];
	// Dereference the register
	const uint16_t memory_address = beemu_registers_read_16(device->registers, register_);
	if (from_accum)
	{
		// Write to memory from Accumulator.
		beemu_memory_write(device->memory,
						   memory_address,
						   beemu_registers_read_8(device->registers,
												  BEEMU_REGISTER_A));
	}
	else
	{
		// Write to the ACCUMULATOR from memory.
		beemu_registers_write_8(device->registers,
								BEEMU_REGISTER_A,
								beemu_memory_read(device->memory,
												  memory_address));
	}
	// Then, decrement or increment HL if need be.
	if (device->current_instruction.first_nibble == 0x20)
	{
		beemu_registers_increment_16(device->registers, BEEMU_REGISTER_HL);
	}
	else if (device->current_instruction.first_nibble == 0x0A)
	{
		beemu_registers_arithmatic_16_unary(device->registers, BEEMU_REGISTER_HL, BEEMU_UOP_DEC);
	}
}

/**
 * @brief Decode condition for JP, JR, CALL or RET
 *
 * Some instruction have condition to decide whether or not
 * to execute.
 * @param device
 * @return BeemuJumpCondition
 */
BeemuJumpCondition decide_condition(BeemuDevice *device)
{
	const bool is_no_condition = beemu_util_is_one_of(device->current_instruction.instruction, 6, 0x18, 0xC3, 0xF9, 0xCD, 0xD9, 0xE9);
	BeemuJumpCondition condition = BEEMU_JUMP_IF_NO_CONDITION;
	if (!is_no_condition)
	{
		switch (device->current_instruction.instruction)
		{
		case 0x20:
		case 0xC0:
		case 0xC2:
		case 0xC4:
			condition = BEEMU_JUMP_IF_NOT_ZERO;
			break;
		case 0x30:
		case 0xD0:
		case 0xD2:
		case 0xD4:
			condition = BEEMU_JUMP_IF_NOT_CARRY;
			break;
		case 0x28:
		case 0xC8:
		case 0xCA:
		case 0xCC:
			condition = BEEMU_JUMP_IF_ZERO;
			break;
		case 0x38:
		case 0xD8:
		case 0xDA:
		case 0xDC:
			condition = BEEMU_JUMP_IF_CARRY;
			break;
		}
	}
	return condition;
}

/**
 * @brief Execute a JP or JR instruction.
 *
 * Execute a JP or JR instruction, decide the correct parameters
 * based on the instruction.
 * @param device
 */
void execute_jump(BeemuDevice *device)
{
	const bool is_jr = device->current_instruction.first_nibble < 0x70;
	BeemuJumpCondition condition = decide_condition(device);
	uint16_t value = 0;
	if (is_jr)
	{
		// In JR we have a 1 byte data.
		pop_data(device, true);
		value = device->data.data_8;
	}
	else
	{
		// In JP we have a 2 byte data.
		pop_data(device, false);
		value = device->data.data_16;
	}
	beemu_registers_jump(device->registers, condition,
						 value, !is_jr, device->current_instruction.instruction == 0xE9);
}

/**
 * @brief Execute one of the rotate.
 *
 * Execute RLCA, RLA, RRCA or RRA instructions.
 * @param device BeemuDevice object pointer.
 */
void execute_rotate_a(BeemuDevice *device)
{
	const bool rotate_right = device->current_instruction.second_nibble = 0x0F;
	const bool through_c = device->current_instruction.first_nibble = 0x00;
	beemu_registers_rotate_A(device->registers, rotate_right, through_c);
}

/**
 * @brief Load SP to memory.
 *
 * Load SP to the memory address provided in
 * the data.
 * @param device BeemuDevice object pointer.
 */
void execute_load_sp_to_mem(BeemuDevice *device)
{
	pop_data(device, false);
	const uint16_t address = device->data.data_16;
	// Get PC value.
	uint16_t value = beemu_registers_read_16(device->registers, BEEMU_REGISTER_PC);
	beemu_memory_write_16(device->memory, address, value);
}

/**
 * @brief Execute an instruction that will either set
 * or complement the carry flag.
 *
 * @param device BeemuDevice object pointer.
 */
void execute_set_complement_flag(BeemuDevice *device)
{
	// Use symmetry to determine which one to execute.
	const bool is_set = device->current_instruction.second_nibble == 0x07;
	// Set H and N flags.
	beemu_registers_flag_set(device->registers, BEEMU_FLAG_H, false);
	beemu_registers_flag_set(device->registers, BEEMU_FLAG_N, false);
	if (is_set)
	{
		beemu_registers_flag_set(device->registers, BEEMU_FLAG_C, true);
	}
	else
	{
		const bool previous_value = beemu_registers_flag_is_high(device->registers, BEEMU_FLAG_C);
		beemu_registers_flag_set(device->registers, BEEMU_FLAG_C, !previous_value);
	}
}

/**
 * @brief Push a value into the stack.
 *
 * Push a value into a stack, decrementing the SP twice.
 * @param device BeemuDevice object pointer.
 * @param value Value to push into the stack.
 */
void push_stack(BeemuDevice *device, uint16_t value)
{
	const uint16_t stack_pointer = beemu_registers_read_16(device->registers, BEEMU_REGISTER_SP);
	beemu_memory_write_16(device->memory, stack_pointer, value);
	beemu_registers_decrement_16(device->registers, BEEMU_REGISTER_SP);
	beemu_registers_decrement_16(device->registers, BEEMU_REGISTER_SP);
}

/**
 * @brief Pop a value from the stack.
 *
 * Pop a value from the stack and increment the SP twice.
 * @param device BeemuDevice object pointer.
 * @return uint16_t Value in the stack.
 */
uint16_t pop_stack(BeemuDevice *device)
{
	const uint16_t stack_pointer = beemu_registers_read_16(device->registers, BEEMU_REGISTER_SP);
	const uint16_t value = beemu_memory_read_16(device->memory, stack_pointer);
	beemu_registers_increment_16(device->registers, BEEMU_REGISTER_SP);
	beemu_registers_increment_16(device->registers, BEEMU_REGISTER_SP);
	return value;
}

/**
 * @brief Execute a stack operation.
 *
 * Operating on the stack portion of the memory execute
 * a POP or PUSH operation based on the SP register.
 * @param device
 */
void execute_stack_op(BeemuDevice *device, BeemuStackOperation operation)
{
	const uint16_t stack_pointer = beemu_registers_read_16(device->registers, BEEMU_REGISTER_PC);
	const int index = (device->current_instruction.first_nibble - 0xC0) >> 1;
	const BeemuRegister_16 target_register = ORDERED_REGISTER_STACK_NAMES_16[index];
	switch (operation)
	{
	case BEEMU_SOP_POP:
	{
		uint16_t value = pop_stack(device);
		beemu_registers_write_16(device->registers, target_register, value);
		break;
	}
	case BEEMU_SOP_PUSH:
	{
		uint16_t value = beemu_registers_read_16(device->registers, target_register);
		push_stack(device, value);
		break;
	}
	}
}

/**
 * @brief Execute a "partial" load.
 *
 * Execute a load from/to $FF00 + Register
 * @param device
 */
void execute_load_partial(BeemuDevice *device)
{
	const uint8_t address_lower = beemu_registers_read_8(device->registers, BEEMU_REGISTER_C);
	const uint8_t address = beemu_util_combine_8_to_16(0xFF, address_lower);
	if (device->current_instruction.first_nibble == 0xE0)
	{
		// Load from A
		const uint8_t register_value = beemu_registers_read_8(device->registers, BEEMU_REGISTER_A);
		beemu_memory_write(device->memory, address, register_value);
	}
	else
	{
		// Load to A
		const uint8_t memory_value = beemu_memory_read(device->memory, address);
		beemu_registers_write_8(device->registers, BEEMU_REGISTER_A, memory_value);
	}
}

/**
 * @brief Execute a RST instruction.
 *
 * Push the current address to the stack and jump to
 * an address close to 0x0000.
 * @param device BeemuDevice object pointer.
 */
void execute_reset_instruction(BeemuDevice *device)
{
	const uint16_t offset = device->current_instruction.first_nibble - 0xC0;
	const uint16_t base = device->current_instruction.second_nibble == 0x7E ? 0x00 : 0x08;
	const uint16_t new_addres = base + offset;
	const uint16_t current_address = beemu_registers_read_16(device->registers, BEEMU_REGISTER_PC);
	// Push the current address to the stack.
	push_stack(device, current_address);
	beemu_registers_jump(device->registers, BEEMU_JUMP_IF_NO_CONDITION,
						 new_addres, true, false);
}

/**
 * @brief Execute a call instruction
 *
 * Execute a CALL instruction.
 * @param device BeemuDevice object pointer.
 */
void execute_call_instruction(BeemuDevice *device)
{
	BeemuJumpCondition condition = decide_condition(device);
	const uint16_t current_address = beemu_registers_read_16(device->registers, BEEMU_REGISTER_PC);
	push_stack(device, current_address + 2);
	// Get the jump address.
	pop_data(device, false);
	beemu_registers_jump(device->registers, condition, device->data.data_16, true, false);
}

/**
 * @brief Load from/to A to/from a dereferenced memory.
 *
 * @param device BeemuDevice object pointer.
 * @param from_accumulator If true, get value from A.
 */
void execute_load_A_dereference(BeemuDevice *device, bool from_accumulator)
{
	pop_data(device, false);
	if (from_accumulator)
	{
		const uint8_t accumulator_value = beemu_registers_read_8(device->registers,
																 BEEMU_REGISTER_A);
		const uint16_t memory_address = beemu_memory_read_16(device->memory, device->data.data_16);
		beemu_memory_write(device->memory, memory_address, accumulator_value);
	}
	else
	{
		const uint8_t value_at_memory = beemu_memory_read(device->memory, device->data.data_16);
		beemu_registers_write_8(device->registers, BEEMU_REGISTER_A, value_at_memory);
	}
}

/**
 * @brief Load from/to A to/from a dereferenced memory with offset.
 *
 * @param device BeemuDevice object pointer.
 * @param from_accumulator If true, get value from A.
 */
void execute_ldh(BeemuDevice *device, bool from_accumulator)
{
	pop_data(device, true);
	const uint16_t address = 0xFF00 + ((uint16_t)device->data.data_8);
	if (from_accumulator)
	{
		const uint8_t current_value = beemu_registers_read_8(device->registers, BEEMU_REGISTER_A);
		beemu_memory_write(device->memory, address, current_value);
	}
	else
	{
		const uint8_t memory_value = beemu_memory_read(device->memory, address);
		beemu_registers_write_8(device->registers, BEEMU_REGISTER_A, memory_value);
	}
}

void execute_ret(BeemuDevice *device)
{
	const BeemuJumpCondition condition = decide_condition(device);
	const uint16_t memory_address = pop_stack(device);
	beemu_registers_jump(device->registers, condition, memory_address,
						 true, false);
	if (device->current_instruction.instruction == 0xDA)
	{
		device->interrupts_enabled = true;
	}
}

/**
 * @brief Process the device state.
 *
 * Process the device state, such as awaits for
 * interrupt disable and enable.
 * @param device
 */
void process_device_state(BeemuDevice *device)
{
	switch (device->device_state)
	{
	case BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE:
		device->interrupts_enabled = false;
		beemu_device_set_state(device, BEEMU_DEVICE_NORMAL);
		break;
	case BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE:
		device->interrupts_enabled = true;
		beemu_device_set_state(device, BEEMU_DEVICE_NORMAL);
		break;
	default:
		break;
	}
}

void execute_cb_prefix(BeemuDevice *device)
{
}

/**
 * @brief Execute an instruction from the CF block.
 *
 * CF block comprimises those instructions from 0xC0 to
 * 0xFF.
 * @param device BeemuDevice object pointer.
 */
void execute_cf_block(BeemuDevice *device)
{
	switch (device->current_instruction.second_nibble)
	{
	case 0x00:
		if (device->current_instruction.second_nibble >= 0xE0)
		{
			execute_ldh(device, device->current_instruction.instruction == 0xE0);
		}
		execute_ret(device);
		break;
	case 0x01:
	case 0x05:
		execute_stack_op(device, device->current_instruction.second_nibble == 0x05 ? BEEMU_SOP_PUSH : BEEMU_SOP_POP);
		break;
	case 0x02:
		if (device->current_instruction.first_nibble <= 0xD0)
		{
			// Jump
			execute_jump(device);
		}
		else
		{
			execute_load_partial(device);
		}
	case 0x03:
		if (device->current_instruction.first_nibble == 0xC0)
		{
			execute_jump(device);
		}
		else if (device->current_instruction.first_nibble == 0xF0)
		{
			beemu_device_set_state(device, BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE);
		}
		break;
	case 0x04:
	case 0x0C:
	case 0x0D:
		if (device->current_instruction.first_nibble >= 0xE0 || device->current_instruction.instruction == 0xDD)
		{
			break;
		}
		execute_call_instruction(device);
		break;
	case 0x06:
	case 0x07:
	case 0x0F:
		execute_reset_instruction(device);
		break;
	case 0x08:
	case 0x09:
		if (device->current_instruction.first_nibble <= 0xD0)
		{
			execute_ret(device);
		}
		else if (device->current_instruction.instruction == 0xE9)
		{
			execute_jump(device);
		}
		break;
	case 0x0A:
		if (device->current_instruction.second_nibble <= 0xD0)
		{
			execute_jump(device);
		}
		else
		{
			execute_load_A_dereference(device, device->current_instruction.first_nibble == 0xE0);
		}
		break;
	case 0x0B:
		switch (device->current_instruction.first_nibble)
		{
		case 0xC0:
			execute_cb_prefix(device);
			break;
		case 0xF0:
			beemu_device_set_state(device, BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE);
			break;
		default:
			break;
		}
		break;
	case 0x0E:
		execute_arithmatic_register_instruction(device);
		break;
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
		case 0x10:
			beemu_device_set_state(device, BEEMU_DEVICE_STOP);
			break;
		case 0x20:
		case 0x30:
			execute_jump(device);
			break;
		default:
			break;
		}
	case 0x01:
		execute_load_direct(device, false);
		break;
	case 0x02:
		execute_load_accumulator_16(device, true);
		break;
	case 0x0A:
		execute_load_accumulator_16(device, false);
		break;
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
	case 0x07:
	case 0x0F:
		switch (device->current_instruction.first_nibble)
		{
		case 0x00:
		case 0x10:
			execute_rotate_a(device);
			break;
		case 0x20:
			if (device->current_instruction.second_nibble == 0x0F)
			{
				beemu_registers_complement_A(device->registers);
			}
			else
			{
				beemu_registers_BCD(device->registers);
			}
			break;
		case 0x30:
			execute_set_complement_flag(device);
			break;
		}
	case 0x08:
		switch (device->current_instruction.first_nibble)
		{
		case 0x00:
			execute_load_sp_to_mem(device);
			break;
		case 0x10:
		case 0x20:
		case 0x30:
			execute_jump(device);
			break;
		}
	case 0x09:
		execute_arithmatic_register_instruction_16(device);
		break;
	default:
		break;
	}
}

void beemu_device_run(BeemuDevice *device)
{
	process_device_state(device);
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
		case 0xC0:
		case 0xD0:
		case 0xE0:
		case 0xF0:
			execute_cf_block(device);
		}
	}
loop_stop:
	// Use a label here to avoid checking the conditions all the time.
	return;
}
