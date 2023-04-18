#include <libbeemu/device/processor/registers.h>
#include <libbeemu/internals/utility.h>
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

BeemuRegister_16 mappable_16_bit_registers[3] = {BEEMU_REGISTER_BC, BEEMU_REGISTER_DE, BEEMU_REGISTER_HL};

BeemuRegister_8 beemu_16_to_8_bit_converter[3][2] = {
	{BEEMU_REGISTER_B, BEEMU_REGISTER_C},
	{BEEMU_REGISTER_D, BEEMU_REGISTER_E},
	{BEEMU_REGISTER_H, BEEMU_REGISTER_L}};

/**
 * @brief Read an 8 bit register.
 *
 * @param registers Register file pointer.
 * @param register_ Register to read frmo.
 * @return uint16_t
 */
uint16_t
beemu_read_basic_register(BeemuRegisters *registers, BeemuRegister_8 register_)
{
	return registers->registers[register_];
}

/**
 * @brief Read a 16 bit register that is mapped to two registers.
 *
 * @param registers Register file pointer.
 * @param register_ Register to read frmo.
 * @return uint16_t
 */
uint16_t
beemu_read_composed_register(BeemuRegisters *registers, BeemuRegister_16 register_)
{
	if (register_ == BEEMU_REGISTER_AF)
	{
		// Combine A and flags to a single value
		return beemu_util_combine_8_to_16(beemu_read_basic_register(registers, BEEMU_REGISTER_A), registers->flags);
	}
	int index = 0;
	// Sure, sure this is not extendable,
	// but I don't think GB architecture will change
	// anytime soon.
	if (register_ == BEEMU_REGISTER_DE)
	{
		index = 1;
	}
	else
	{
		index = 2;
	}
	uint16_t left_register_value = beemu_read_basic_register(registers, beemu_16_to_8_bit_converter[index][0]);
	uint16_t right_register_value = beemu_read_basic_register(registers, beemu_16_to_8_bit_converter[index][1]);
	return beemu_util_combine_8_to_16(left_register_value, right_register_value);
}

/**
 * @brief Write to a composed register.
 *
 * @param registers
 * @param register_
 * @return void*
 */
void beemu_write_composed_register(BeemuRegisters *registers, BeemuRegister_16 register_, uint16_t value)
{
	const uint16_t left_value = value >> 8;
	const uint16_t right_value = value & 0x0F;
	if (register_ == BEEMU_REGISTER_AF)
	{
		registers->registers[BEEMU_REGISTER_A] = left_value;
		registers->flags = right_value;
		return;
	}
	int index = 0;
	// Now for the actual compounds.
	if (register_ == BEEMU_REGISTER_DE)
	{
		index = 1;
	}
	else
	{
		index = 2;
	}
	uint16_t left_register = beemu_16_to_8_bit_converter[index][0];
	uint16_t right_register = beemu_16_to_8_bit_converter[index][1];
	registers->registers[left_register] = left_value;
	registers->registers[right_register] = right_register;
}

uint8_t *
beemu_get_register_ptr_8(BeemuRegisters *registers, BeemuRegister_8 register_)
{
	return &registers->registers[register_];
}

/**
 * @brief Read a register ptr from a 16 bit register.
 * Do keep in mind that the ptrs for BC, DE and HL  and AF is not here.
 *
 * @param registers
 * @param register_
 * @return uint16_t
 */
uint16_t *
beemu_get_register_ptr_16(BeemuRegisters *registers, BeemuRegister_16 register_)
{
	switch (register_)
	{
	case BEEMU_REGISTER_M:
		// Pseudo register m.
		return registers->m_register;
	case BEEMU_REGISTER_AF:
		// This one combines the A with flags.
		return beemu_util_combine_8_to_16(beemu_read_basic_register(registers, BEEMU_REGISTER_A), registers->flags);
	case BEEMU_REGISTER_SP:
		return registers->stack_pointer;
	case BEEMU_REGISTER_PC:
		return registers->program_counter;
	}
}

uint16_t beemu_registers_read_register_value(BeemuRegisters *registers, BeemuRegister register_)
{
	if (register_.type == BEEMU_EIGHT_BIT_REGISTER)
	{
		return *beemu_get_register_ptr_8(registers, register_.name_of.eight_bit_register);
	}
	else if (register_.type == BEEMU_SIXTEEN_BIT_REGISTER && beemu_util_is_one_of(register_.name_of.sixteen_bit_register, BEEMU_REGISTER_M, BEEMU_REGISTER_PC, BEEMU_REGISTER_SP))
	{
		return *beemu_get_register_ptr_16(registers, register_.name_of.sixteen_bit_register);
	}
	else
	{
		// Otherwise this is a composed
		return beemu_read_composed_register(registers, register_.name_of.sixteen_bit_register);
	}
}

void beemu_registers_write_register_value(BeemuRegisters *registers, BeemuRegister register_, uint16_t value)
{
	if (register_.type == BEEMU_EIGHT_BIT_REGISTER)
	{
		*beemu_get_register_ptr_8(registers, register_.name_of.eight_bit_register) = value;
	}
	else if (register_.type == BEEMU_SIXTEEN_BIT_REGISTER && beemu_util_is_one_of(register_.name_of.sixteen_bit_register, BEEMU_REGISTER_M, BEEMU_REGISTER_PC, BEEMU_REGISTER_SP))
	{
		*beemu_get_register_ptr_16(registers, register_.name_of.sixteen_bit_register) = value;
	}
	else
	{
		// Otherwise this is a composed
		beemu_write_composed_register(registers, register_.name_of.sixteen_bit_register, value);
	}
}

void beemu_registers_flags_set_flag(BeemuRegisters *registers, BeemuFlag flag, uint8_t value)
{
	// This should probably work.
	registers->flags |= value << flag;
}
