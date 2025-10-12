#include <stdlib.h>
#include <beemu/device/processor/processor.h>

BeemuProcessor *beemu_processor_new(void)
{
	BeemuProcessor *processor = (BeemuProcessor *)malloc(sizeof(BeemuProcessor));
	processor->memory = beemu_memory_new(BEEMU_DEVICE_MEMORY_SIZE);
	processor->registers = beemu_registers_new();
	processor->interrupts_enabled = true;
	processor->processor_state = BEEMU_DEVICE_NORMAL;
	BeemuRegister pc_register = {.type = BEEMU_SIXTEEN_BIT_REGISTER,
								 .name_of = {.sixteen_bit_register = BEEMU_REGISTER_PC}};
	beemu_registers_write_register_value(processor->registers, pc_register, BEEMU_DEVICE_MEMORY_ROM_LOCATION);
	return processor;
}

void beemu_processor_free(BeemuProcessor *processor)
{
	beemu_memory_free(processor->memory);
	beemu_registers_free(processor->registers);
	free(processor);
}

/**
 * @brief Set the elapsed clock cycle count for the processor.
 *
 * @param processor BeemuProcessor object pointer.
 * @param value Value to be set.
 */
static inline void beemu_processor_set_elapsed_clock_cycle(BeemuProcessor *processor, uint8_t value)
{
	processor->elapsed_clock_cycle = value;
}

/**
 * @brief Reset the processor's elapsed clock cycle to 0.
 *
 * @param processor BeemuProcessor object pointer.
 */
static inline void beemu_processor_reset_elapsed_clock_cycle(BeemuProcessor *processor)
{
	beemu_processor_set_elapsed_clock_cycle(processor, 0);
}

BeemuProcessorState beemu_processor_get_processor_state(BeemuProcessor *processor)
{
	return processor->processor_state;
}

void beemu_processor_set_state(BeemuProcessor *processor, BeemuProcessorState state)
{
	processor->processor_state = state;
}

uint8_t beemu_processor_run(BeemuProcessor *processor)
{
	return 0x0;
	// execute_instruction(processor->memory, processor->registers, ...);
}
