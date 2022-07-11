#include <stdlib.h>
#include <device/processor.h>
#include <device/memory.h>
#include <internals/utility.h>

BeemuProcessor *beemu_processor_new(void)
{
	BeemuProcessor *processor = (BeemuProcessor *)malloc(sizeof(BeemuProcessor));
	processor->memory = beemu_memory_new(BEEMU_DEVICE_MEMORY_SIZE);
	processor->registers = beemu_registers_new();
	processor->interrupts_enabled = true;
	processor->processor_state = BEEMU_DEVICE_NORMAL;
	beemu_registers_write_16(processor->registers, BEEMU_REGISTER_PC, BEEMU_DEVICE_MEMORY_ROM_LOCATION);
	return processor;
}

void beemu_processor_free(BeemuProcessor *processor)
{
	beemu_memory_free(processor->memory);
	beemu_registers_free(processor->registers);
	free(processor);
}

BeemuProcessorState beemu_processor_get_processor_state(BeemuProcessor *processor)
{
	return processor->processor_state;
}

void beemu_processor_set_state(BeemuProcessor *processor, BeemuProcessorState state)
{
	processor->processor_state = state;
}

/**
 * @brief Peek the next instruction.
 *
 * Get the next instruction to be executed.
 * @param processor
 */
static inline uint8_t peek_instruction(BeemuProcessor *processor)
{
	const uint16_t next_instruction_location = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
	const uint8_t next_instruction = beemu_memory_read(processor->memory, next_instruction_location);
	return next_instruction;
}

/**
 * @brief Pop the next instruction.
 *
 * Get the next instruction to be executed, increment
 * the program counter.
 * @param processor
 */
static inline uint16_t pop_instruction(BeemuProcessor *processor)
{
	const uint8_t next_instruction = peek_instruction(processor);
	processor->current_instruction.instruction = next_instruction;
	processor->current_instruction.first_nibble = next_instruction & 0xF0;
	processor->current_instruction.second_nibble = next_instruction & 0x0F;
	beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_PC);
	return next_instruction;
}

/**
 * @brief Pop and load data to processor from memory.
 *
 * @param processor BeemuProcessor object pointer.
 */
static inline void pop_data(BeemuProcessor *processor, bool is_byte_length)
{
	if (is_byte_length)
	{
		const uint8_t data = peek_instruction(processor);
		// Increment the PC register.
		beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_PC);
		processor->data.data_8 = data;
	}
	else
	{
		// For 16 bit data.
		const uint8_t high = peek_instruction(processor);
		beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_PC);
		const uint8_t lower = peek_instruction(processor);
		beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_PC);
		processor->data.data_16 = (((uint16_t)high) << 8) | ((uint16_t)lower);
	}
}

/**
 * @brief Dereference the HL register.
 *
 * Get the value of the HL register and then return the value
 * that is inside the memory address at that value.
 * @param processor BeemuProcessor object.
 * @return uint8_t the dereferenced HL register.
 */
uint8_t dereference_hl(BeemuProcessor *processor)
{
	const uint16_t memory_address = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_HL);
	const uint8_t dereferenced_value = beemu_memory_read(processor->memory, memory_address);
	return dereferenced_value;
}

/**
 * @brief Write to the memory location at the (HL)
 *
 * Dereference HL and write to the memory location it points to.
 * @param processor BeemuProcessor object pointer.
 * @param value Value to write.
 */
void write_to_dereferenced_hl(BeemuProcessor *processor, uint8_t value)
{
	const uint16_t memory_address = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_HL);
	beemu_memory_write(processor->memory, memory_address, value);
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
 * @param processor BeemuProcessor object.
 */
static inline void execute_arithmatic_register_instruction(BeemuProcessor *processor)
{
	BeemuOperation operation = BEEMU_OP_ADD;
	switch (processor->current_instruction.first_nibble)
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
		if (processor->current_instruction.second_nibble >= 0x08)
		{
			operation = BEEMU_OP_XOR;
		}
		break;
	case 0xB0:
	case 0xF0:
		operation = BEEMU_OP_OR;
		if (processor->current_instruction.second_nibble >= 0x08)
		{
			operation = BEEMU_OP_CP;
		}
		break;
	}
	const bool should_add_carry = processor->current_instruction.second_nibble > 0x08 && processor->current_instruction.first_nibble >= 0xA0;
	if (processor->current_instruction.first_nibble >= 0xC0)
	{
		pop_data(processor, true);
		beemu_registers_arithmatic_8_constant(processor->registers,
											  processor->data.data_8,
											  operation,
											  processor->current_instruction.instruction == 0xCE && processor->current_instruction.instruction == 0xDE);
	}
	else if (processor->current_instruction.second_nibble == 0x06 || processor->current_instruction.second_nibble == 0x0E)
	{
		// This uses the dereferenced HL register.
		const uint8_t dereferenced_value = dereference_hl(processor);
		beemu_registers_arithmatic_8_constant(processor->registers,
											  dereferenced_value,
											  operation,
											  should_add_carry);
	}
	else
	{
		beemu_registers_arithmatic_8_register(processor->registers,
											  decode_register_from_instruction(processor->current_instruction.instruction),
											  operation,
											  should_add_carry);
	}
}

/**
 * @brief Execute a load instruction.
 *
 * @param processor BeemuProcessor object.
 */
static inline void execute_load_instruction(BeemuProcessor *processor)
{
	const BeemuRegister_8 destination_register = decode_register_from_instruction(processor->current_instruction.instruction);
	uint8_t value = dereference_hl(processor);
	if (!(processor->current_instruction.first_nibble == 0x70 && processor->current_instruction.second_nibble <= 0x07))
	{
		// If outside this block, then read from a register.
		const BeemuRegister_8 source_register = decode_ld_register_from_instruction(processor->current_instruction.instruction);
		value = beemu_registers_read_8(processor->registers, source_register);
	}
	if (processor->current_instruction.second_nibble != 0x06 && processor->current_instruction.second_nibble != 0x0E)
	{
		// Load to the dereferenced (HL) memory location.
		beemu_memory_write(processor->memory, dereference_hl(processor), value);
	}
	else
	{
		// Otherwise laod to the destination register.
		beemu_registers_write_8(processor->registers, destination_register, value);
	}
}

/**
 * @brief Execute unary inc or dec on memory.
 *
 * Execute an increment or a decrement operation on the
 * value of a memory address dereferenced by HL and
 * write it to the same memory, set the flags.
 * @param processor BeemuProcessor object pointer.
 * @param operation Operation to perform.
 */
void execute_unary_on_memory(BeemuProcessor *processor, BeemuUnaryOperation operation)
{
	uint8_t previous_value = dereference_hl(processor);
	const uint8_t new_value = operation == BEEMU_UOP_INC ? previous_value + 1 : previous_value - 1;
	// Binary equivalent of the unary operation, INC => ADD, DEC => SUB.
	const uint8_t op_equivalent = operation == BEEMU_UOP_INC ? BEEMU_OP_ADD : BEEMU_OP_SUB;
	write_to_dereferenced_hl(processor, new_value);
	beemu_registers_set_flags(processor->registers, previous_value, new_value, false, op_equivalent, true);
}

/**
 * @brief Decode 03 Block's symmetric register encodings.
 *
 * Some instructions like the LD direct, INC and DEC instructions of
 * the 03 block has symmetrical encoding of registers depending on whether
 * or not the second nibble is lower than 0x07.
 * @param processor BeemuProcessor object pointer.
 * @return BeemuRegister_8 the decoded register.
 */
BeemuRegister_8 decode_03_block_sym_register(BeemuProcessor *processor)
{
	const bool on_the_left = processor->current_instruction.second_nibble < 0x07;
	const int row = on_the_left ? 0 : 1;
	const int column = processor->current_instruction.first_nibble >> 1;
	return SYMMETRIC_REGISTERS[column][row];
}

/**
 * @brief Execute a register increment or decrement operation.
 *
 * Execute a register increment or decrement operation, the register to
 * operate on depends on the instruction.
 * @param processor BeemuProcessor object pointer.
 * @param increment If set to true, increment, otherwise decrement.
 */
void execute_unary_operand(BeemuProcessor *processor, bool increment)
{
	const bool on_the_left = processor->current_instruction.second_nibble < 0x07;
	const BeemuUnaryOperation uop = increment ? BEEMU_UOP_INC : BEEMU_UOP_DEC;
	if (on_the_left && processor->current_instruction.first_nibble == 0x30)
	{
		// These act on the dereferenced HL.
		execute_unary_on_memory(processor, uop);
	}
	else
	{
		beemu_registers_arithmatic_8_unary(processor->registers,
										   decode_03_block_sym_register(processor),
										   uop);
	}
}

/**
 * @brief Execute a register increment or decrement operation on 16.
 *
 * Execute a register increment or decrement operation, the register to
 * operate on depends on the instruction.
 * @param processor BeemuProcessor object pointer.
 */
void execute_unary_operand_16(BeemuProcessor *processor)
{
	const bool on_the_left = processor->current_instruction.second_nibble < 0x07;
	const int index = processor->current_instruction.first_nibble >> 1;
	const BeemuUnaryOperation uop = on_the_left ? BEEMU_UOP_INC : BEEMU_UOP_DEC;
	beemu_registers_arithmatic_16_unary(processor->registers, ORDERED_REGISTER_NAMES_16[index], uop);
}

/**
 * @brief Execute an arithmatic register instruction
 *
 * Execute ADD with 16 bitregisters
 * affecting the HL.
 *
 * @param processor BeemuProcessor object.
 */
static inline void execute_arithmatic_register_instruction_16(BeemuProcessor *processor)
{
	// Only supported 16 bit OP is ADD (for some reason.).
	BeemuOperation operation = BEEMU_OP_ADD;
	const int index = processor->current_instruction.first_nibble >> 1;
	beemu_registers_arithmatic_16_register(processor->registers,
										   ORDERED_REGISTER_NAMES_16[index],
										   operation);
}

/**
 * @brief Load directly to a register
 *
 * @param processor BeemuProcessor object pointer.
 * @param is_byte_length If set to true, interpret as a load to
 * a 8 register.
 */
void execute_load_direct(BeemuProcessor *processor, bool is_byte_length)
{
	if (is_byte_length)
	{
		// Load 8 bit data.
		pop_data(processor, true);
		const BeemuRegister_8 destination_register = decode_03_block_sym_register(processor);
		beemu_registers_write_8(processor->registers, destination_register, processor->data.data_8);
	}
	else
	{
		// Load 16 bit data
		pop_data(processor, false);
		const BeemuRegister_16 destination_register = ORDERED_REGISTER_NAMES_16[processor->current_instruction.first_nibble >> 1];
		beemu_registers_write_16(processor->registers, destination_register, processor->data.data_16);
	}
}

/**
 * @brief Execute LD to/from dereferenced memory addresses to/from accumulator.
 *
 * @param processor BeemuProcessor object pointer.
 * @param from_accum if set to true, load from accumulator to the
 * dereferenced memory location, otherwise, load from the dereferenced
 * memory location to the accumulator register.
 */
static void execute_load_accumulator_16(BeemuProcessor *processor, bool from_accum)
{
	// Finally, you may have to decrement or increment.
	static const BeemuRegister_16 registers[4] = {
		BEEMU_REGISTER_BC,
		BEEMU_REGISTER_DE,
		BEEMU_REGISTER_HL,
		BEEMU_REGISTER_HL};
	const BeemuRegister_16 register_ = registers[register_];
	// Dereference the register
	const uint16_t memory_address = beemu_registers_read_16(processor->registers, register_);
	if (from_accum)
	{
		// Write to memory from Accumulator.
		beemu_memory_write(processor->memory,
						   memory_address,
						   beemu_registers_read_8(processor->registers,
												  BEEMU_REGISTER_A));
	}
	else
	{
		// Write to the ACCUMULATOR from memory.
		beemu_registers_write_8(processor->registers,
								BEEMU_REGISTER_A,
								beemu_memory_read(processor->memory,
												  memory_address));
	}
	// Then, decrement or increment HL if need be.
	if (processor->current_instruction.first_nibble == 0x20)
	{
		beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_HL);
	}
	else if (processor->current_instruction.first_nibble == 0x0A)
	{
		beemu_registers_arithmatic_16_unary(processor->registers, BEEMU_REGISTER_HL, BEEMU_UOP_DEC);
	}
}

/**
 * @brief Decode condition for JP, JR, CALL or RET
 *
 * Some instruction have condition to decide whether or not
 * to execute.
 * @param processor
 * @return BeemuJumpCondition
 */
BeemuJumpCondition decide_condition(BeemuProcessor *processor)
{
	const bool is_no_condition = beemu_util_is_one_of(processor->current_instruction.instruction, 6, 0x18, 0xC3, 0xF9, 0xCD, 0xD9, 0xE9);
	BeemuJumpCondition condition = BEEMU_JUMP_IF_NO_CONDITION;
	if (!is_no_condition)
	{
		switch (processor->current_instruction.instruction)
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
 * @param processor
 */
void execute_jump(BeemuProcessor *processor)
{
	const bool is_jr = processor->current_instruction.first_nibble < 0x70;
	BeemuJumpCondition condition = decide_condition(processor);
	uint16_t value = 0;
	if (is_jr)
	{
		// In JR we have a 1 byte data.
		pop_data(processor, true);
		value = processor->data.data_8;
	}
	else
	{
		// In JP we have a 2 byte data.
		pop_data(processor, false);
		value = processor->data.data_16;
	}
	beemu_registers_jump(processor->registers, condition,
						 value, !is_jr, processor->current_instruction.instruction == 0xE9);
}

/**
 * @brief Execute one of the rotate.
 *
 * Execute RLCA, RLA, RRCA or RRA instructions.
 * @param processor BeemuProcessor object pointer.
 */
void execute_rotate_a(BeemuProcessor *processor)
{
	const bool rotate_right = processor->current_instruction.second_nibble = 0x0F;
	const bool through_c = processor->current_instruction.first_nibble = 0x00;
	beemu_registers_rotate_A(processor->registers, rotate_right, through_c);
}

/**
 * @brief Load SP to memory.
 *
 * Load SP to the memory address provided in
 * the data.
 * @param processor BeemuProcessor object pointer.
 */
void execute_load_sp_to_mem(BeemuProcessor *processor)
{
	pop_data(processor, false);
	const uint16_t address = processor->data.data_16;
	// Get PC value.
	uint16_t value = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
	beemu_memory_write_16(processor->memory, address, value);
}

/**
 * @brief Execute an instruction that will either set
 * or complement the carry flag.
 *
 * @param processor BeemuProcessor object pointer.
 */
void execute_set_complement_flag(BeemuProcessor *processor)
{
	// Use symmetry to determine which one to execute.
	const bool is_set = processor->current_instruction.second_nibble == 0x07;
	// Set H and N flags.
	beemu_registers_flag_set(processor->registers, BEEMU_FLAG_H, false);
	beemu_registers_flag_set(processor->registers, BEEMU_FLAG_N, false);
	if (is_set)
	{
		beemu_registers_flag_set(processor->registers, BEEMU_FLAG_C, true);
	}
	else
	{
		const bool previous_value = beemu_registers_flag_is_high(processor->registers, BEEMU_FLAG_C);
		beemu_registers_flag_set(processor->registers, BEEMU_FLAG_C, !previous_value);
	}
}

/**
 * @brief Push a value into the stack.
 *
 * Push a value into a stack, decrementing the SP twice.
 * @param processor BeemuProcessor object pointer.
 * @param value Value to push into the stack.
 */
void push_stack(BeemuProcessor *processor, uint16_t value)
{
	const uint16_t stack_pointer = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_SP);
	beemu_memory_write_16(processor->memory, stack_pointer, value);
	beemu_registers_decrement_16(processor->registers, BEEMU_REGISTER_SP);
	beemu_registers_decrement_16(processor->registers, BEEMU_REGISTER_SP);
}

/**
 * @brief Pop a value from the stack.
 *
 * Pop a value from the stack and increment the SP twice.
 * @param processor BeemuProcessor object pointer.
 * @return uint16_t Value in the stack.
 */
uint16_t pop_stack(BeemuProcessor *processor)
{
	const uint16_t stack_pointer = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_SP);
	const uint16_t value = beemu_memory_read_16(processor->memory, stack_pointer);
	beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_SP);
	beemu_registers_increment_16(processor->registers, BEEMU_REGISTER_SP);
	return value;
}

/**
 * @brief Execute a stack operation.
 *
 * Operating on the stack portion of the memory execute
 * a POP or PUSH operation based on the SP register.
 * @param processor
 */
void execute_stack_op(BeemuProcessor *processor, BeemuStackOperation operation)
{
	const uint16_t stack_pointer = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
	const int index = (processor->current_instruction.first_nibble - 0xC0) >> 1;
	const BeemuRegister_16 target_register = ORDERED_REGISTER_STACK_NAMES_16[index];
	switch (operation)
	{
	case BEEMU_SOP_POP:
	{
		uint16_t value = pop_stack(processor);
		beemu_registers_write_16(processor->registers, target_register, value);
		break;
	}
	case BEEMU_SOP_PUSH:
	{
		uint16_t value = beemu_registers_read_16(processor->registers, target_register);
		push_stack(processor, value);
		break;
	}
	}
}

/**
 * @brief Execute a "partial" load.
 *
 * Execute a load from/to $FF00 + Register
 * @param processor
 */
void execute_load_partial(BeemuProcessor *processor)
{
	const uint8_t address_lower = beemu_registers_read_8(processor->registers, BEEMU_REGISTER_C);
	const uint8_t address = beemu_util_combine_8_to_16(0xFF, address_lower);
	if (processor->current_instruction.first_nibble == 0xE0)
	{
		// Load from A
		const uint8_t register_value = beemu_registers_read_8(processor->registers, BEEMU_REGISTER_A);
		beemu_memory_write(processor->memory, address, register_value);
	}
	else
	{
		// Load to A
		const uint8_t memory_value = beemu_memory_read(processor->memory, address);
		beemu_registers_write_8(processor->registers, BEEMU_REGISTER_A, memory_value);
	}
}

/**
 * @brief Execute a RST instruction.
 *
 * Push the current address to the stack and jump to
 * an address close to 0x0000.
 * @param processor BeemuProcessor object pointer.
 */
void execute_reset_instruction(BeemuProcessor *processor)
{
	const uint16_t offset = processor->current_instruction.first_nibble - 0xC0;
	const uint16_t base = processor->current_instruction.second_nibble == 0x7E ? 0x00 : 0x08;
	const uint16_t new_addres = base + offset;
	const uint16_t current_address = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
	// Push the current address to the stack.
	push_stack(processor, current_address);
	beemu_registers_jump(processor->registers, BEEMU_JUMP_IF_NO_CONDITION,
						 new_addres, true, false);
}

/**
 * @brief Execute a call instruction
 *
 * Execute a CALL instruction.
 * @param processor BeemuProcessor object pointer.
 */
void execute_call_instruction(BeemuProcessor *processor)
{
	BeemuJumpCondition condition = decide_condition(processor);
	const uint16_t current_address = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
	push_stack(processor, current_address + 2);
	// Get the jump address.
	pop_data(processor, false);
	beemu_registers_jump(processor->registers, condition, processor->data.data_16, true, false);
}

/**
 * @brief Load from/to A to/from a dereferenced memory.
 *
 * @param processor BeemuProcessor object pointer.
 * @param from_accumulator If true, get value from A.
 */
void execute_load_A_dereference(BeemuProcessor *processor, bool from_accumulator)
{
	pop_data(processor, false);
	if (from_accumulator)
	{
		const uint8_t accumulator_value = beemu_registers_read_8(processor->registers,
																 BEEMU_REGISTER_A);
		const uint16_t memory_address = beemu_memory_read_16(processor->memory, processor->data.data_16);
		beemu_memory_write(processor->memory, memory_address, accumulator_value);
	}
	else
	{
		const uint8_t value_at_memory = beemu_memory_read(processor->memory, processor->data.data_16);
		beemu_registers_write_8(processor->registers, BEEMU_REGISTER_A, value_at_memory);
	}
}

/**
 * @brief Load from/to A to/from a dereferenced memory with offset.
 *
 * @param processor BeemuProcessor object pointer.
 * @param from_accumulator If true, get value from A.
 */
void execute_ldh(BeemuProcessor *processor, bool from_accumulator)
{
	pop_data(processor, true);
	const uint16_t address = 0xFF00 + ((uint16_t)processor->data.data_8);
	if (from_accumulator)
	{
		const uint8_t current_value = beemu_registers_read_8(processor->registers, BEEMU_REGISTER_A);
		beemu_memory_write(processor->memory, address, current_value);
	}
	else
	{
		const uint8_t memory_value = beemu_memory_read(processor->memory, address);
		beemu_registers_write_8(processor->registers, BEEMU_REGISTER_A, memory_value);
	}
}

void execute_ret(BeemuProcessor *processor)
{
	const BeemuJumpCondition condition = decide_condition(processor);
	const uint16_t memory_address = pop_stack(processor);
	beemu_registers_jump(processor->registers, condition, memory_address,
						 true, false);
	if (processor->current_instruction.instruction == 0xDA)
	{
		processor->interrupts_enabled = true;
	}
}

/**
 * @brief Add a signed integer to the SP.
 *
 * @param processor
 */
void execute_add_sp_r8(BeemuProcessor *processor)
{
	const uint16_t value = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_SP);
	pop_data(processor, true);
	uint8_t new_value = processor->data.data_8 + value;
	if ((processor->data.data_8 & 0x80) > 0)
	{
		// Since this is a signed integer first get the pure integer.
		const uint8_t effective_value = 0x7F & processor->data.data_8;
		new_value = value - ((uint16_t)effective_value);
	}
	beemu_registers_set_flags(processor->registers, value, new_value, false, BEEMU_OP_ADD,
							  false);
	beemu_registers_write_16(processor->registers, BEEMU_REGISTER_SP, new_value);
}

/**
 * @brief Execute LD HL, SP related instructions.
 *
 * These instructions load SP to HL or vice versa,
 * sometimes with an addition to SP value first.
 * @param processor
 */
void execute_ldhl_sp(BeemuProcessor *processor)
{
	if (processor->current_instruction.instruction == 0xF8)
	{
		// Signed addition.
		pop_data(processor, true);
		const uint16_t old_value = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
		uint16_t new_value = old_value + (uint8_t)processor->data.data_8;
		if ((processor->data.data_8 & 0x80) > 0)
		{
			// Negative number.
			const uint8_t effective_value = processor->data.data_8 & 0x7F;
			new_value = old_value - ((uint16_t)effective_value);
		}
		const uint16_t old_value_hl = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_HL);
		beemu_registers_set_flags(processor->registers, old_value_hl, new_value,
								  false, BEEMU_OP_ADD, false);
		beemu_registers_write_16(processor->registers, BEEMU_REGISTER_HL, new_value);
	}
	else
	{
		// F9
		const uint16_t sp_value = beemu_registers_read_16(processor->registers, BEEMU_REGISTER_PC);
		beemu_registers_write_16(processor->registers, BEEMU_REGISTER_HL, sp_value);
	}
}

/**
 * @brief Process the processor state.
 *
 * Process the processor state, such as awaits for
 * interrupt disable and enable.
 * @param processor
 */
void process_processor_state(BeemuProcessor *processor)
{
	switch (processor->processor_state)
	{
	case BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE:
		processor->interrupts_enabled = false;
		beemu_processor_set_state(processor, BEEMU_DEVICE_NORMAL);
		break;
	case BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE:
		processor->interrupts_enabled = true;
		beemu_processor_set_state(processor, BEEMU_DEVICE_NORMAL);
		break;
	default:
		break;
	}
}

void execute_cb_prefix(BeemuProcessor *processor)
{
}

/**
 * @brief Execute an instruction from the CF block.
 *
 * CF block comprimises those instructions from 0xC0 to
 * 0xFF.
 * @param processor BeemuProcessor object pointer.
 */
void execute_cf_block(BeemuProcessor *processor)
{
	switch (processor->current_instruction.second_nibble)
	{
	case 0x00:
		if (processor->current_instruction.second_nibble >= 0xE0)
		{
			execute_ldh(processor, processor->current_instruction.instruction == 0xE0);
		}
		execute_ret(processor);
		break;
	case 0x01:
	case 0x05:
		execute_stack_op(processor, processor->current_instruction.second_nibble == 0x05 ? BEEMU_SOP_PUSH : BEEMU_SOP_POP);
		break;
	case 0x02:
		if (processor->current_instruction.first_nibble <= 0xD0)
		{
			// Jump
			execute_jump(processor);
		}
		else
		{
			execute_load_partial(processor);
		}
	case 0x03:
		if (processor->current_instruction.first_nibble == 0xC0)
		{
			execute_jump(processor);
		}
		else if (processor->current_instruction.first_nibble == 0xF0)
		{
			beemu_processor_set_state(processor, BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE);
		}
		break;
	case 0x04:
	case 0x0C:
	case 0x0D:
		if (processor->current_instruction.first_nibble >= 0xE0 || processor->current_instruction.instruction == 0xDD)
		{
			break;
		}
		execute_call_instruction(processor);
		break;
	case 0x06:
	case 0x07:
	case 0x0F:
		execute_reset_instruction(processor);
		break;
	case 0x08:
	case 0x09:
		switch (processor->current_instruction.instruction)
		{
		case 0xC8:
		case 0xD8:
		case 0xC9:
		case 0xD9:
			execute_ret(processor);
			break;
		case 0xE9:
			execute_jump(processor);
			break;
		case 0xE8:
			execute_add_sp_r8(processor);
			break;
		case 0xF8:
		case 0xF9:
			execute_ldhl_sp(processor);
			break;
		}
		break;
	case 0x0A:
		if (processor->current_instruction.second_nibble <= 0xD0)
		{
			execute_jump(processor);
		}
		else
		{
			execute_load_A_dereference(processor, processor->current_instruction.first_nibble == 0xE0);
		}
		break;
	case 0x0B:
		switch (processor->current_instruction.first_nibble)
		{
		case 0xC0:
			execute_cb_prefix(processor);
			break;
		case 0xF0:
			beemu_processor_set_state(processor, BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE);
			break;
		default:
			break;
		}
		break;
	case 0x0E:
		execute_arithmatic_register_instruction(processor);
		break;
	}
}

/**
 * @brief Execute the block of instructions between row 0x00 and 0x30.
 *
 * These blocks of instructions display a periodic table like behaviour
 * depending on the last nibble.
 * @param processor BeemuProcessor object pointer.
 */
void execute_block_03(BeemuProcessor *processor)
{
	switch (processor->current_instruction.second_nibble)
	{
	case 0x00:
		switch (processor->current_instruction.first_nibble)
		{
		case 0x00:
			break;
		case 0x10:
			beemu_processor_set_state(processor, BEEMU_DEVICE_STOP);
			break;
		case 0x20:
		case 0x30:
			execute_jump(processor);
			break;
		default:
			break;
		}
	case 0x01:
		execute_load_direct(processor, false);
		break;
	case 0x02:
		execute_load_accumulator_16(processor, true);
		break;
	case 0x0A:
		execute_load_accumulator_16(processor, false);
		break;
	case 0x04:
	case 0x05:
	case 0x0C:
	case 0x0D:
		// INC and DEC
		execute_unary_operand(processor, processor->current_instruction.second_nibble == 0x04 || processor->current_instruction.second_nibble == 0x0C);
		break;
	case 0x06:
	case 0x0E:
		execute_load_direct(processor, true);
		break;
	case 0x07:
	case 0x0F:
		switch (processor->current_instruction.first_nibble)
		{
		case 0x00:
		case 0x10:
			execute_rotate_a(processor);
			break;
		case 0x20:
			if (processor->current_instruction.second_nibble == 0x0F)
			{
				beemu_registers_complement_A(processor->registers);
			}
			else
			{
				beemu_registers_BCD(processor->registers);
			}
			break;
		case 0x30:
			execute_set_complement_flag(processor);
			break;
		}
	case 0x08:
		switch (processor->current_instruction.first_nibble)
		{
		case 0x00:
			execute_load_sp_to_mem(processor);
			break;
		case 0x10:
		case 0x20:
		case 0x30:
			execute_jump(processor);
			break;
		}
	case 0x09:
		execute_arithmatic_register_instruction_16(processor);
		break;
	default:
		break;
	}
}

void beemu_processor_run(BeemuProcessor *processor)
{
	while (true)
	{
		process_processor_state(processor);
		uint8_t next_instruction = pop_instruction(processor);
		switch (processor->current_instruction.first_nibble)
		{
		case 0x00:
		case 0x10:
		case 0x20:
		case 0x30:
			execute_block_03(processor);
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
			if (processor->current_instruction.second_nibble == 0x06)
			{
				// HALT
				beemu_processor_set_state(processor, BEEMU_DEVICE_HALT);
			}
			else
			{
				execute_load_instruction(processor);
			}
			break;
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
			execute_arithmatic_register_instruction(processor);
			break;
		case 0xC0:
		case 0xD0:
		case 0xE0:
		case 0xF0:
			execute_cf_block(processor);
		}
	}
loop_stop:
	// Use a label here to avoid checking the conditions all the time.
	return;
}
