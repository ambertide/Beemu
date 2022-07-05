#ifndef BEEMU_MEMORY_H
#define BEEMU_MEMORY_H
#include <stdint.h>
#include <stdbool.h>

typedef struct BeemuMemory
{
	int memory_size;
	uint8_t *memory;
} BeemuMemory;

/**
 * @brief Create a memory object.
 *
 * Initialise a memory object of constant size.
 * @param memory_size Size of the memory.
 * @return Beemu_Memory* Pointer to the newly created object.
 */
BeemuMemory *beemu_memory_new(int memory_size);

/**
 * @brief Destroy the memory object.
 *
 * Free the memory allocated to the memory.
 * @param memory Memory to be freed.
 */
void beemu_memory_free(BeemuMemory *memory);

/**
 * @brief Read value at address.
 *
 * Get the value at the given memory address.
 * @param memory pointer to the BeemuMemory object to be read from.
 * @param address address to be read.
 * @return uint8_t Value at the address.
 */
uint8_t beemu_memory_read(BeemuMemory *memory, int address);

/**
 * @brief Write value to a memory address
 *
 * @param memory BeemuMemory object to write to
 * @param address Address to write at.
 * @param value value to be written
 */
void beemu_memory_write(BeemuMemory *memory, int address, uint8_t value);

/**
 * @brief Write bulk data to memory.
 *
 * Write bulk data from a buffer to the memory.
 *
 * @param memory BeemuMemory object to write into.
 * @param address Start address of the memory.
 * @param buffer Buffer to copy from.
 * @param size Size of memory to write.
 * @return true If the write operation is successful
 * @return false If the write operation fails, for instance boundaries.
 */
bool beemu_memory_write_buffer(BeemuMemory *memory, int address, uint8_t *buffer, int size);

/**
 * @brief Read from memory to a buffer.
 *
 * Read from the memory to a buffer, indicate whether or not the
 * memory read has succeeded, failures may occur due to size overflow.
 * @param memory BeemuMemory object to read from
 * @param address Address to read from
 * @param buffer Buffer to write to.
 * @param size Size of the write operation.
 * @return true if the write operation succeeds
 * @return false if the write operation fails.
 */
bool beemu_memory_read_buffer(BeemuMemory *memory, int address, uint8_t *buffer, int size);

/**
 * @brief Copy a block of memory.
 *
 * Copy a block of memory from one BeemuMemory object to another.
 *
 * @param memory Origin BeemuMemory
 * @param destination Target BeemuMemory
 * @param start start address of the memory copy.
 * @param dst_start start address of the memory copy destination.
 * @param size Length of the memory copy.
 */
bool beemu_memory_copy(BeemuMemory *memory, BeemuMemory *destination, int start, int dst_start, int size);

#endif // BEEMU_MEMORY_H
