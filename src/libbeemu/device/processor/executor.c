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
int resolve_param(BeemuRegisters *registers, BeemuMemory *memory, BeemuParam param)
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
void write_to_param(BeemuRegisters *registers, BeemuMemory *memory, BeemuParam param, uint16_t value, bool is_eight_bit_write)
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
	if (is_eight_bit_write)
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
	write_to_param(registers, memory, instruction.params.load_params.dest, load_value, instruction.type == BEEMU_INSTRUCTION_TYPE_LOAD_8);
}

/**
 * @brief Resolve a value to adjust to a given type.
 *
 * Calculate underflows, overflows, and positive adjustments as well.
 * @param num
 * @param paramType
 * @return int
 */
uint16_t resolve_flows(int value, BeemuParamType resolve_to)
{
	switch (resolve_to)
	{
	case BEEMU_PARAM_TYPE_INT_8:
	case BEEMU_PARAM_TYPE_UINT_8:
	case BEEMU_PARAM_TYPE_REGISTER_8:
		return (uint8_t)value;
	case BEEMU_PARAM_TYPE_UINT16:
	case BEEMU_PARAM_TYPE_REGISTER_16:
		return (uint16_t)value;
	default:
		return value;
	}
}

void execute_arithmatic(BeemuRegisters *registers, BeemuMemory *memory, BeemuInstruction instruction)
{
	// First we need to resolve the operands.
	const int operand_one = resolve_param(registers, memory, instruction.params.arithmatic_params.dest);
	const int operand_two = resolve_param(registers, memory, instruction.params.arithmatic_params.source);
	int result = operand_one;
	// Do not assign the result.
	bool silent = false;
	// Then we need to take our operation upon it.
	switch (instruction.params.arithmatic_params.operation)
	{
	case BEEMU_OP_ADD:
		result += operand_two;
		break;
	case BEEMU_OP_AND:
		result &= operand_two;
		break;
	case BEEMU_OP_CP:
		silent = true;
	case BEEMU_OP_SUB:
		result -= operand_two;
		break;
	case BEEMU_OP_XOR:
		result ^= operand_two;
		break;
	case BEEMU_OP_OR:
		result |= operand_two;
		break;
	}
	// Great, now we will do a few things, first,
	// we need to properly encode the result
	// including the over/undef-flows.
	uint16_t resolved_result = resolve_flows(result, instruction.params.arithmatic_params.dest.type);
	// Calculate the flag states.
	beemu_registers_flags_set_flag(registers, BEEMU_FLAG_Z, resolved_result == 0);
	beemu_registers_flags_set_flag(registers, BEEMU_FLAG_C, resolved_result != result);
	// Write the actual result.
	if (!silent)
	{
		write_to_param(
			registers,
			memory,
			instruction.params.arithmatic_params.dest,
			resolved_result,
			beemu_util_is_one_of_two(
				instruction.params.arithmatic_params.dest.type,
				BEEMU_PARAM_TYPE_REGISTER_8,
				BEEMU_PARAM_TYPE_UINT_8));
	}
}

bool test_condition(BeemuRegisters *registers, BeemuJumpCondition condition)
{
	const uint8_t zero = beemu_registers_flags_get_flag(registers, BEEMU_FLAG_Z);
	const uint8_t carry = beemu_registers_flags_get_flag(registers, BEEMU_FLAG_C);
	switch (condition)
	{
	case BEEMU_JUMP_IF_CARRY:
		return carry == 1;
	case BEEMU_JUMP_IF_NOT_CARRY:
		return carry == 0;
	case BEEMU_JUMP_IF_ZERO:
		return zero == 1;
	case BEEMU_JUMP_IF_NOT_ZERO:
		return zero == 0;
	default:
		return true;
	}
}

/**
 * @brief Push to the stack a 16 bit value.
 *
 * @param registers
 * @param memory
 * @param value
 */
void push_stack(BeemuRegisters *registers, BeemuMemory *memory, uint16_t value)
{
	static const BeemuRegister sp = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_PC};
	const uint16_t current_sp = beemu_registers_read_register_value(registers, sp);
	beemu_registers_write_register_value(registers, sp, current_sp - 2);
	beemu_memory_write_16(memory, current_sp - 2, current_sp);
}

/**
 * @brief Pop from the stack a 16 bit value.
 *
 * @param registers
 * @param memory
 * @return uint16_t
 */
uint16_t pop_stack(BeemuRegisters *registers, BeemuMemory *memory)
{
	static const BeemuRegister sp = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_SP};
	const uint16_t current_sp = beemu_registers_read_register_value(registers, sp);
	const uint16_t val = beemu_memory_read_16(memory, current_sp);
	beemu_registers_write_register_value(registers, sp, current_sp + 2);
	return val;
}

/**
 * @brief Execute a JUMP instruction
 *
 * This is a CALL, RET, JR, JMP or RST instruction.
 *
 * @param memory
 * @param registers
 * @param instruction
 */
void execute_jump(BeemuMemory *memory, BeemuRegisters *registers, BeemuInstruction instruction)
{
	static const BeemuRegister pc = {.type = BEEMU_SIXTEEN_BIT_REGISTER, .name_of.sixteen_bit_register = BEEMU_REGISTER_PC};
	uint16_t current_address = registers->program_counter;
	BeemuJumpParams params = instruction.params.jump_params;
	// Could be a pointer, relative or direct addr.
	const int addr_param = resolve_param(registers, memory, params.param);
	if (params.is_conditional && !test_condition(registers, params.condition))
	{
		// Conditional jumps that do not fit the conditions are not executed.
		return;
	}
	// Either non-conditional or true cond.
	uint16_t new_address = 0x0; // RST
	if (params.is_relative)
	{
		new_address = current_address + addr_param;
	}
	else if (params.type == BEEMU_JUMP_TYPE_RET)
	{
		// It could also be a RET jump too,
		// so the addr is determined by the stack.
		new_address = pop_stack(registers, memory);
	}
	else
	{
		// Finally, it could be a direct jump.
		// or a RST.
		new_address = addr_param;
	}

	if (params.type == BEEMU_JUMP_TYPE_CALL)
	{
		// Though, if it is a call, some extra
		// stuff is needed.
		// Store current address.
		push_stack(registers, memory, current_address);
	}

	// We can finally, actually, jump.
	beemu_registers_write_register_value(registers, pc, new_address);
	// TODO: Enable interrupts.
}

uint8_t execute_instruction(BeemuMemory *memory, BeemuRegisters *file, BeemuInstruction instruction)
{
	switch (instruction.type)
	{
	case BEEMU_INSTRUCTION_TYPE_LOAD_8:
	case BEEMU_INSTRUCTION_TYPE_LOAD_16:
		execute_load(file, memory, instruction);
		break;
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8:
	case BEEMU_INSTRUCTION_TYPE_ARITHMATIC_16:
		execute_arithmatic(file, memory, instruction);
		break;
	case BEEMU_INSTRUCTION_TYPE_JUMP:
		execute_jump(memory, file, instruction);
		break;
	}

	return instruction.duration_in_clock_cycles;
}
