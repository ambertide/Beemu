#include <libbeemu/device/processor/executor.h>
#include <libbeemu/device/primitives/register.h>
#include <libbeemu/internals/utility.h>

/**
 * @brief Resolve a param's value.
 *
 * A param's value depends on multiple factors,
 * its type, and whether or not it is a pointer.
 *
 * When a parameter is a pointer, its value is
 * dereferenced by taking the value of the memory
 * by interpreting the value as a memory address.
 *
 * Register parameters are resolved to the value
 * of the respective register referred.
 *
 * @param registers Register file pointer.
 * @param memory Memory pointer.
 * @param param Parameter to be resolved.
 * @return int Integer value.
 */
int resolve_param(BeemuMemory *memory, BeemuRegisters *registers, BeemuParam param)
{
	BeemuRegister register_;
	uint16_t param_value = 0;
	switch (param.type)
	{
	case BEEMU_PARAM_TYPE_REGISTER_8:
		register_.type = BEEMU_EIGHT_BIT_REGISTER;
		register_.name_of.eight_bit_register = param.value.register_8;
		param_value = beemu_registers_read_register_value(registers, register_);
		break;
	case BEEMU_PARAM_TYPE_REGISTER_16:
		register_.type = BEEMU_SIXTEEN_BIT_REGISTER;
		register_.name_of.sixteen_bit_register = param.value.register_16;
		param_value = beemu_registers_read_register_value(registers, register_);
		break;
	case BEEMU_PARAM_TYPE_UINT16:
	case BEEMU_PARAM_TYPE_UINT_8:
		// Both are hold in a uint16_t
		param_value = param.value.value;
		break;
	case BEEMU_PARAM_TYPE_INT_8:
		// This one is signed.
		param_value = param.value.signed_value;
		break;
	}

	// Dereference to memory if exists and if is a pointer.
	if (param.pointer)
	{
		return beemu_memory_read(memory, param_value);
	}
	else
	{
		// Otherwise return value itself.
		return param_value;
	}
}

/**
 * @brief Write to... somewhere, dependening on a param.
 *
 * @param memory Memory object pointer.
 * @param registers Register file pointer.
 * @param param Parameter to dictate where to write.
 * @param value Value to write.
 * @param type Type of the location.
 */
void write_to_param(BeemuMemory *memory, BeemuRegisters *registers, BeemuParam param, uint16_t value, BeemuInstructionType type)
{
	BeemuRegister register_;
	uint16_t write_location;
	const bool is_register = beemu_util_is_one_of_two(param.type, BEEMU_PARAM_TYPE_REGISTER_8, BEEMU_PARAM_TYPE_REGISTER_16);
	if (is_register)
	{
		// Set the register type regardless.
		register_.type = param.type == BEEMU_PARAM_TYPE_REGISTER_8 ? BEEMU_EIGHT_BIT_REGISTER : BEEMU_SIXTEEN_BIT_REGISTER;
		if (param.type == BEEMU_PARAM_TYPE_REGISTER_8)
		{
			register_.name_of.eight_bit_register = param.value.register_8;
		}
		else
		{
			register_.name_of.sixteen_bit_register = param.value.register_16;
		}

		// Now check if this is a pointer write.

		if (param.pointer)
		{
			// Then we shall take the register's value and
			// write the value to the THAT address at the memory
			write_location = beemu_registers_read_register_value(registers, register_);
		}
		else
		{
			// By the way, the other variant is much easier anyhow,
			// just go
			beemu_registers_write_register_value(registers, register_, value);
			return;
		}
	}
	else
	{
		// Otherwise the type HAS to be
		// BEEMU_PARAM_TYPE_UINT_8 or BEEMU_PARAM_TYPE_UINT_16
		// and MUST indicate location... I think.
		write_location = param.value.value;
	}

	// Allright, if we got to this point, either we have a valid
	// write location, or we are about to segfault. A logical person
	// would try to do an error check, I am, however, not a logical person
	// nor particularly intelligent, so...
	if (type == BEEMU_INSTRUCTION_TYPE_LOAD_8)
	{
		beemu_memory_write(memory, write_location, value);
	}
	else
	{
		beemu_memory_write_16(memory, write_location, value);
	}
}

/**
 * @brief Execute a single instruction of class LOAD_8 or LOAD_16.
 *
 * @param registers Register file pointer.
 * @param memory Memory pointer.
 * @param instruction Instruction to execute.
 */
void execute_load(BeemuRegisters *registers, BeemuMemory *memory, BeemuInstruction instruction)
{
	const uint16_t load_value = resolve_param(registers, memory, instruction.params.load_params.source);
	write_to_param(memory, registers, instruction.params.load_params.dest, load_value, instruction.type);
}

void execute_instruction(BeemuMemory *memory, BeemuRegisters *file, BeemuInstruction instruction)
{
	switch (instruction.type)
	{
	case BEEMU_INSTRUCTION_TYPE_LOAD_8:
	case BEEMU_INSTRUCTION_TYPE_LOAD_16:
		execute_load(memory, file, instruction);
		break;
	}
}
