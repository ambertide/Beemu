#include <device/registers.h>
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
	}
	case BEEMU_REGISTER_SP:
		registers->stack_pointer = value;
	case BEEMU_REGISTER_PC:
		registers->program_counter = value;
	case BEEMU_REGISTER_M:
		registers->m_register = value;
	default:
		break;
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

void beemu_registers_increment_8(BeemuRegisters *registers, BeemuRegister_8 register_name)
{
	registers->registers[register_name]++;
}

/**
 * @brief Set flags based on registers.
 *
 * Determine and set the flags based on the previous and present
 * value of the registers.
 *
 * @param registers BeemuRegisters object.
 * @param previous_value Previous value of the register.
 * @param next_value Next value of the register.
 * @param after_add_carry First calculate carry, and then add the result to
 * next_value, and only after that determine and calculate the other flags.
 * @param operation Operation that was performed.
 */
void set_flags_register_arithmatic_8(BeemuRegisters *registers, uint8_t previous_value, uint8_t next_value, bool after_add_carry, BeemuOperation operation)
{
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
	}
	beemu_registers_flag_set(registers, BEEMU_FLAG_Z, next_value == 0b0);
	beemu_registers_flag_set(registers, BEEMU_FLAG_N,
							 operation == BEEMU_OP_SUB || operation == BEEMU_OP_CP);
	// TODO: This is almost certainly false.
	beemu_registers_flag_set(registers, BEEMU_FLAG_H,
							 previous_value < 0x0F && next_value > 0x0F);
}

void beemu_registers_arithmatic_8_constant(BeemuRegisters *registers, uint8_t value, BeemuOperation operation, bool should_add_carry)
{
	const uint8_t previous_value = beemu_registers_read_8(registers, BEEMU_REGISTER_A);
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
	set_flags_register_arithmatic_8(registers,
									previous_value,
									operation == BEEMU_OP_CP ? cp_value : final_value,
									should_add_carry,
									operation);
	if (should_add_carry)
	{
		final_value += beemu_registers_flag_read(registers, BEEMU_FLAG_C);
	}
	beemu_registers_write_8(registers, BEEMU_REGISTER_A, final_value);
}

void beemu_registers_arithmatic_8_register(BeemuRegisters *registers, BeemuRegister_8 register_, BeemuOperation operation, bool should_add_carry)
{
	const uint8_t register_value = beemu_registers_read_8(registers, register_);
	beemu_registers_arithmatic_8_constant(registers, register_value, operation, should_add_carry);
}