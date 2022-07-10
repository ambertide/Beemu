#include <device/registers.h>
#include <internals/utility.h>
#include <stdlib.h>

BeemuRegisters *beemu_registers_new(void)
{
	BeemuRegisters *registers = (BeemuRegisters *)malloc(sizeof(BeemuRegisters));
	registers->m_register = 0x0;
	registers->program_counter = 0x0;
	registers->stack_pointer = 0x0;
	registers->flags = 0x0;
	for (int i = 0; i < 7; i++)
	{
		registers->registers[i] = 0x0;
	}
	return registers;
}

void beemu_registers_free(BeemuRegisters *registers)
{
	free(registers);
}

uint8_t beemu_registers_read_8(BeemuRegisters *registers, BeemuRegister_8 register_name)
{
	return registers->registers[register_name];
}

void beemu_registers_write_8(BeemuRegisters *registers, BeemuRegister_8 register_name, uint8_t value)
{
	registers->registers[register_name] = value;
}

void beemu_registers_flags_clear(BeemuRegisters *registers)
{
	registers->flags = 0x00;
}

uint16_t beemu_registers_read_16(BeemuRegisters *registers, BeemuRegister_16 register_name)
{
	switch (register_name)
	{
	case BEEMU_REGISTER_BC:
	case BEEMU_REGISTER_DE:
	case BEEMU_REGISTER_HL:
	{ // Destructure the combined registers into two 8 bit register indices.
		const int stop_register_index = (register_name + 1) * 2;
		const int start_register_index = stop_register_index - 1;
		// Get the values.
		const uint16_t start_register = (uint16_t)registers->registers[start_register_index];
		const uint16_t stop_register = (uint16_t)registers->registers[stop_register_index];
		// Combine the two.
		return (start_register << 8) | stop_register;
	}
	case BEEMU_REGISTER_SP:
		return registers->stack_pointer;
	case BEEMU_REGISTER_PC:
		return registers->program_counter;
	case BEEMU_REGISTER_M:
		return registers->m_register;
	case BEEMU_REGISTER_AF:
		return beemu_util_combine_8_to_16(
			beemu_registers_read_8(registers, BEEMU_REGISTER_A),
			beemu_registers_flag_read_all(registers));
	default:
		break;
	}
}

void beemu_registers_write_16(BeemuRegisters *registers, BeemuRegister_16 register_name, uint16_t value)
{
	switch (register_name)
	{
	case BEEMU_REGISTER_BC:
	case BEEMU_REGISTER_DE:
	case BEEMU_REGISTER_HL:
	{ // Destructure the combined registers into two 8 bit register indices.
		const int stop_register_index = (register_name + 1) * 2;
		const int start_register_index = stop_register_index - 1;
		// Get the values.
		const uint8_t start_register_value = (uint8_t)value >> 8;
		const uint8_t stop_register_value = (uint8_t)value & 0x0F;
		// Combine the two.
		registers->registers[start_register_index] = start_register_value;
		registers->registers[stop_register_index] = stop_register_value;
		break;
	}
	case BEEMU_REGISTER_SP:
		registers->stack_pointer = value;
		break;
	case BEEMU_REGISTER_PC:
		registers->program_counter = value;
		break;
	case BEEMU_REGISTER_M:
		registers->m_register = value;
		break;
	case BEEMU_REGISTER_AF:
	{
		BeemuByteTuple decomposition = beemu_util_split_16_to_8(value);
		beemu_registers_write_8(registers, BEEMU_REGISTER_A, decomposition.first);
		registers->flags = decomposition.second;
		break;
	}
	}
}

uint8_t beemu_registers_flag_read_all(BeemuRegisters *registers)
{
	return registers->flags;
}

uint8_t beemu_registers_flag_read(BeemuRegisters *registers, BeemuFlag flag_name)
{
	const uint8_t value = (registers->flags >> flag_name) & 0x1;
	return value;
}

void beemu_registers_flag_set(BeemuRegisters *registers, BeemuFlag flag_name, bool value)
{
	if (beemu_registers_flag_is_high(registers, flag_name) != value)
	{
		// Otherwise the case where already is 1 and we wish 1 is broken.
		registers->flags ^= 0x1 << flag_name;
	}
}

bool beemu_registers_flag_is_high(BeemuRegisters *registers, BeemuFlag flag_name)
{
	return beemu_registers_flag_read(registers, flag_name) == 0x01;
}

uint16_t beemu_registers_read_psw(BeemuRegisters *registers)
{
	const uint16_t padded_A_register = ((uint16_t)beemu_registers_read_8(registers, BEEMU_REGISTER_A)) << 8;
	const uint16_t flags = (uint16_t)beemu_registers_flag_read_all(registers);
	return padded_A_register | flags;
}

void beemu_registers_increment_16(BeemuRegisters *registers, BeemuRegister_16 register_name)
{
	const uint16_t previous_value = beemu_registers_read_16(registers, register_name);
	beemu_registers_write_16(registers, register_name, previous_value + 1);
}

void beemu_registers_decrement_16(BeemuRegisters *registers, BeemuRegister_16 register_name)
{
	const uint16_t previous_value = beemu_registers_read_16(registers, register_name);
	beemu_registers_write_16(registers, register_name, previous_value - 1);
}

void beemu_registers_increment_8(BeemuRegisters *registers, BeemuRegister_8 register_name)
{
	registers->registers[register_name]++;
}

void beemu_registers_set_flags(BeemuRegisters *registers, uint16_t previous_value, uint16_t next_value, bool after_add_carry, BeemuOperation operation, bool is_byte_length)
{
	// DO NOT CHANGE THIS TO INT BECAUSE THE NEXT_VALUE + 1 CARRY WONT WORK.
	if (operation == BEEMU_OP_ADD)
	{
		// Carry is actually an overflow flag.
		beemu_registers_flag_set(registers, BEEMU_FLAG_C, next_value < previous_value);
	}
	else if (operation == BEEMU_OP_SUB)
	{
		// Probably the underflow.
		beemu_registers_flag_set(registers, BEEMU_FLAG_C, next_value > previous_value);
	}
	// Now we check if carry should be added to the next value.
	if (after_add_carry)
	{
		next_value += beemu_registers_flag_read(registers, BEEMU_FLAG_C);
		if (is_byte_length)
		{
			next_value = (uint8_t)next_value;
		}
	}
	beemu_registers_flag_set(registers, BEEMU_FLAG_Z, next_value == 0b0);
	beemu_registers_flag_set(registers, BEEMU_FLAG_N,
							 operation == BEEMU_OP_SUB || operation == BEEMU_OP_CP);
	// TODO: This is almost certainly false.
	beemu_registers_flag_set(registers, BEEMU_FLAG_H,
							 previous_value < 0x0F && next_value > 0x0F);
}

/**
 * @brief Perform an operation on a register.
 *
 * Perform an operation on a register and store the result on
 * the same register.
 * @param registers BeemuRegisters object pointer.
 * @param target_register Target register to operate on.
 * @param value Value to use.
 * @param operation Operation to perform.
 * @param should_add_carry If set to true, add the carry to the result.
 */
void register_perform_operation(BeemuRegisters *registers, BeemuRegister_8 target_register, uint8_t value, BeemuOperation operation, bool should_add_carry)
{
	const uint8_t previous_value = beemu_registers_read_8(registers, target_register);
	uint8_t final_value = 0;
	const uint8_t cp_value = previous_value - value;
	switch (operation)
	{
	case BEEMU_OP_ADD:
		final_value = value + previous_value;
		break;
	case BEEMU_OP_AND:
		final_value = value & previous_value;
		break;
	case BEEMU_OP_OR:
		final_value = value | previous_value;
		break;
	case BEEMU_OP_SUB:
		final_value = previous_value - value;
		break;
	case BEEMU_OP_CP:
		final_value = previous_value;
		break;
	case BEEMU_OP_XOR:
		final_value = previous_value ^ value;
	}
	beemu_registers_set_flags(registers,
							  previous_value,
							  operation == BEEMU_OP_CP ? cp_value : final_value,
							  should_add_carry,
							  operation,
							  true);
	if (should_add_carry)
	{
		final_value += beemu_registers_flag_read(registers, BEEMU_FLAG_C);
	}
	beemu_registers_write_8(registers, target_register, final_value);
}

/**
 * @brief Perform an operation on a 16-bit register.
 *
 * Perform an operation on a register and store the result on
 * the same register.
 * @param registers BeemuRegisters object pointer.
 * @param target_register Target register to operate on.
 * @param value Value to use.
 * @param operation Operation to perform.
 * @param should_add_carry If set to true, add the carry to the result.
 */
void register_perform_operation_16(BeemuRegisters *registers, BeemuRegister_16 target_register, uint16_t value, BeemuOperation operation, bool should_add_carry)
{
	const uint16_t previous_value = beemu_registers_read_16(registers, target_register);
	uint16_t final_value = 0;
	const uint16_t cp_value = previous_value - value;
	switch (operation)
	{
	case BEEMU_OP_ADD:
		final_value = value + previous_value;
		break;
	case BEEMU_OP_AND:
		final_value = value & previous_value;
		break;
	case BEEMU_OP_OR:
		final_value = value | previous_value;
		break;
	case BEEMU_OP_SUB:
		final_value = previous_value - value;
		break;
	case BEEMU_OP_CP:
		final_value = previous_value;
		break;
	case BEEMU_OP_XOR:
		final_value = previous_value ^ value;
	}
	beemu_registers_set_flags(registers,
							  previous_value,
							  operation == BEEMU_OP_CP ? cp_value : final_value,
							  should_add_carry,
							  operation,
							  false);
	if (should_add_carry)
	{
		final_value += beemu_registers_flag_read(registers, BEEMU_FLAG_C);
	}
	beemu_registers_write_16(registers, target_register, final_value);
}

void beemu_registers_arithmatic_8_constant(BeemuRegisters *registers, uint8_t value, BeemuOperation operation, bool should_add_carry)
{
	register_perform_operation(registers, BEEMU_REGISTER_A, value, operation, should_add_carry);
}

void beemu_registers_arithmatic_16_constant(BeemuRegisters *registers, uint16_t value, BeemuOperation operation)
{
	register_perform_operation_16(registers, BEEMU_REGISTER_HL, value, operation, false);
}

void beemu_registers_arithmatic_8_register(BeemuRegisters *registers, BeemuRegister_8 register_, BeemuOperation operation, bool should_add_carry)
{
	const uint8_t register_value = beemu_registers_read_8(registers, register_);
	beemu_registers_arithmatic_8_constant(registers, register_value, operation, should_add_carry);
}

void beemu_registers_arithmatic_16_register(BeemuRegisters *registers, BeemuRegister_16 register_, BeemuOperation operation)
{
	const uint16_t register_value = beemu_registers_read_16(registers, register_);
	beemu_registers_arithmatic_16_constant(registers, register_value, operation);
}

void beemu_registers_arithmatic_8_unary(BeemuRegisters *registers, BeemuRegister_8 register_, BeemuUnaryOperation operation)
{
	register_perform_operation(registers, register_, 1, operation == BEEMU_UOP_INC ? BEEMU_OP_ADD : BEEMU_OP_SUB, false);
}

void beemu_registers_arithmatic_16_unary(BeemuRegisters *registers, BeemuRegister_16 register_, BeemuUnaryOperation operation)
{
	register_perform_operation_16(registers, register_, 1, operation == BEEMU_UOP_INC ? BEEMU_OP_ADD : BEEMU_OP_SUB, false);
}

void beemu_registers_jump(BeemuRegisters *registers, BeemuJumpCondition condition, uint16_t value, bool jump_directly, bool jump_to_hl)
{
	bool should_jump = false;
	// Set the should jump.
	switch (condition)
	{
	case BEEMU_JUMP_IF_NO_CONDITION:
		should_jump = true;
		break;
	case BEEMU_JUMP_IF_CARRY:
		should_jump = beemu_registers_flag_is_high(registers, BEEMU_FLAG_C);
		break;
	case BEEMU_JUMP_IF_NOT_CARRY:
		should_jump = !beemu_registers_flag_is_high(registers, BEEMU_FLAG_C);
		break;
	case BEEMU_JUMP_IF_ZERO:
		should_jump = beemu_registers_flag_is_high(registers, BEEMU_FLAG_Z);
		break;
	case BEEMU_JUMP_IF_NOT_ZERO:
		should_jump = !beemu_registers_flag_is_high(registers, BEEMU_FLAG_Z);
		break;
	default:
		break;
	}
	uint16_t memory_address = jump_directly ? value : beemu_registers_read_16(registers, BEEMU_REGISTER_PC) + value;
	if (jump_directly)
	{
		memory_address = beemu_registers_read_16(registers, BEEMU_REGISTER_HL);
	}
	if (should_jump)
	{
		beemu_registers_write_16(registers, BEEMU_REGISTER_PC, memory_address - 1);
	}
}

void beemu_registers_rotate_A(BeemuRegisters *registers, bool rotate_right, bool through_c)
{
	const uint8_t old_value_of_A = beemu_registers_read_8(registers, BEEMU_REGISTER_A);
	const uint8_t old_value_of_c = beemu_registers_flag_read(registers, BEEMU_FLAG_C);
	uint8_t new_value_of_A = (old_value_of_A << 1);
	uint8_t new_value_of_C = old_value_of_A & 0x01;
	if (through_c)
	{
		new_value_of_A |= old_value_of_c;
	}
	if (rotate_right)
	{
		new_value_of_A >>= 1;
		new_value_of_C = (0x80 & old_value_of_A) >> 7;
		if (through_c)
		{
			new_value_of_A |= (old_value_of_c << 7);
		}
	}
	// RLC and RLA, different from the others, also clear the flags.
	beemu_registers_flags_clear(registers);
	// Set the carry flag.
	beemu_registers_flag_set(registers, BEEMU_FLAG_C, new_value_of_C != 0x00);
}

void beemu_registers_complement_A(BeemuRegisters *registers)
{
	beemu_registers_flag_set(registers, BEEMU_FLAG_N, true);
	beemu_registers_flag_set(registers, BEEMU_FLAG_H, true);
	uint8_t previous_value = beemu_registers_read_8(registers, BEEMU_REGISTER_A);
	beemu_registers_write_8(registers, BEEMU_REGISTER_A, ~previous_value);
}

void beemu_registers_BCD(BeemuRegisters *registers)
{
	// Ported from this answer https://forums.nesdev.org/viewtopic.php?p=196282&sid=f56e78a62f1251b73e054cc7c8465f25#p196282
	// from the nesdev forums by the user AW3.
	uint8_t previous_value = beemu_registers_read_8(registers, BEEMU_REGISTER_A);
	// note: assumes a is a uint8_t and wraps from 0xff to 0
	if (!beemu_registers_flag_is_high(registers, BEEMU_FLAG_N))
	{ // after an addition, adjust if (half-)carry occurred or if result is out of bounds
		if (beemu_registers_flag_is_high(registers, BEEMU_FLAG_C) || previous_value > 0x99)
		{
			previous_value += 0x60;
			beemu_registers_flag_set(registers, BEEMU_FLAG_C, true);
		}
		if (beemu_registers_flag_is_high(registers, BEEMU_FLAG_H) || (previous_value & 0x0f) > 0x09)
		{
			previous_value += 0x6;
		}
	}
	else
	{ // after a subtraction, only adjust if (half-)carry occurred
		if (beemu_registers_flag_is_high(registers, BEEMU_FLAG_C))
		{
			previous_value -= 0x60;
		}
		if (beemu_registers_flag_is_high(registers, BEEMU_FLAG_H))
		{
			previous_value -= 0x6;
		}
	}
	// these flags are always updated
	beemu_registers_flag_set(registers, BEEMU_FLAG_Z, previous_value == 0); // the usual z flag
	beemu_registers_flag_set(registers, BEEMU_FLAG_H, false);				// h flag is always cleared
}

void beemu_registers_stack_pop(BeemuRegisters *registers, BeemuRegister_16 register_, uint16_t value)
{
	beemu_registers_write_16(registers, register_, value);
	beemu_registers_increment_16(registers, BEEMU_REGISTER_SP);
	beemu_registers_increment_16(registers, BEEMU_REGISTER_SP);
}

uint16_t beemu_registers_stack_push(BeemuRegisters *registers, BeemuRegister_16 register_)
{
	const uint16_t value = beemu_registers_read_16(registers, register_);
	beemu_registers_decrement_16(registers, BEEMU_REGISTER_SP);
	beemu_registers_decrement_16(registers, BEEMU_REGISTER_SP);
	return value;
}
