#ifndef BEEMU_DEVICE_PROCESSOR_H
#define BEEMU_DEVICE_PROCESSOR_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>
#include "registers.h"
#include "executor.h"
#include "../memory.h"

	static const BeemuRegister_8 ORDERED_REGISTER_NAMES[8] = {BEEMU_REGISTER_B,
															  BEEMU_REGISTER_C,
															  BEEMU_REGISTER_D,
															  BEEMU_REGISTER_E,
															  BEEMU_REGISTER_H,
															  BEEMU_REGISTER_L,
															  BEEMU_REGISTER_A,
															  BEEMU_REGISTER_A};
	static const BeemuRegister_16 ORDERED_REGISTER_NAMES_16[4] = {
		BEEMU_REGISTER_BC,
		BEEMU_REGISTER_DE,
		BEEMU_REGISTER_HL,
		BEEMU_REGISTER_SP};

	static const BeemuRegister_16 ORDERED_REGISTER_STACK_NAMES_16[4] = {
		BEEMU_REGISTER_BC,
		BEEMU_REGISTER_DE,
		BEEMU_REGISTER_HL,
		BEEMU_REGISTER_AF};

	/**
	 * @brief Used to distinguish symmetrical registers such as used by
	 * 03 Block's LD, INC and DEC.
	 */
	static const BeemuRegister_8 SYMMETRIC_REGISTERS[4][2] = {{BEEMU_REGISTER_B, BEEMU_REGISTER_C}, {BEEMU_REGISTER_D, BEEMU_REGISTER_E}, {BEEMU_REGISTER_H, BEEMU_REGISTER_L}, {BEEMU_REGISTER_A, BEEMU_REGISTER_A}};

	/**
	 * @brief Size of the chip memory.
	 */
	static const int BEEMU_DEVICE_MEMORY_SIZE = 65536;
	/**
	 * @brief Location of the ROM in the main memory.
	 */
	static const int BEEMU_DEVICE_MEMORY_ROM_LOCATION;

	/**
	 * @brief Describes the processor state.
	 *
	 */
	typedef enum BeemuProcessorState
	{
		BEEMU_DEVICE_NORMAL,
		BEEMU_DEVICE_HALT,
		BEEMU_DEVICE_STOP,
		BEEMU_DEVICE_AWAITING_INTERRUPT_DISABLE,
		BEEMU_DEVICE_AWAITING_INTERRUPT_ENABLE
	} BeemuProcessorState;

	typedef struct BeemuProcessor
	{
		BeemuRegisters *registers;
		BeemuMemory *memory;
		BeemuProcessorState processor_state;
		bool interrupts_enabled;
		uint8_t elapsed_clock_cycle;
	} BeemuProcessor;

	/**
	 * @brief Create a new BeemuProcessor instance.
	 *
	 * Create a new BeemuProcessor and return its pointer.
	 * @return BeemuProcessor* pointer to the newly created processor object.
	 */
	BeemuProcessor *beemu_processor_new(void);

	/**
	 * @brief Free the previously created processor.
	 *
	 * @param processor Device to free.
	 */
	void beemu_processor_free(BeemuProcessor *processor);

	/**
	 * @brief Load ROM data to processor.
	 *
	 * Load a GameBoy ROM to the processor memory.
	 * @param processor BeemuProcessor instance to load the ROM.
	 * @param rom ROM data to be loaded.
	 * @return bool Whether or not the load succeeded.
	 */
	bool beemu_processor_load(BeemuProcessor *processor, uint8_t *rom);

	/**
	 * @brief Run the processor for a single instruction.
	 *
	 * Run the data loaded at the ROM section of the memory for a single
	 * instruction and return the elapsed clock cycle count.
	 *
	 * @param processor BeemuProcessor object pointer.
	 * @return the elapsed clock cycle count.
	 */
	uint8_t beemu_processor_run(BeemuProcessor *processor);

	/**
	 * @brief Get the state of the processor.
	 *
	 * Get the current state of the processor.
	 * @param processor BeemuProcessor object pointer.
	 * @return BeemuProcessorState
	 */
	BeemuProcessorState beemu_processor_get_processor_state(BeemuProcessor *processor);

	/**
	 * @brief Set the state of the processor.
	 *
	 * Set the state of the processor.
	 * @param processor BeemuProcessor object pointer.
	 * @param state The new state of the processor.
	 */
	void beemu_processor_set_state(BeemuProcessor *processor, BeemuProcessorState state);
#ifdef __cplusplus
}
#endif
#endif // BEEMU_DEVICE_PROCESSOR_H
