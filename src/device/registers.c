#include <device/registers.h>
#include <stdio.h>

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
