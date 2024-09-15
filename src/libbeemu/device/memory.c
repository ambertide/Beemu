#include <libbeemu/device/memory.h>
#include <libbeemu/internals/logger.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

BeemuMemory *beemu_memory_new(int size)
{
	BeemuMemory *memory = (BeemuMemory *)malloc(sizeof(BeemuMemory));
	memory->memory_size = size;
	memory->memory = (uint8_t *)calloc(memory->memory_size, sizeof(uint8_t));
	return memory;
}

void beemu_memory_free(BeemuMemory *memory)
{
	free(memory->memory);
	free(memory);
}

uint8_t beemu_memory_read(BeemuMemory *memory, int address)
{
	assert(memory->memory_size > address);
	return memory->memory[address];
}

void beemu_memory_write(BeemuMemory *memory, int address, uint8_t value)
{
	assert(memory->memory_size > address);
	memory->memory[address] = value;
}

bool beemu_memory_write_buffer(BeemuMemory *memory, int address, uint8_t *buffer, int size)
{
	if (memory->memory_size <= address + size - 1)
	{
		beemu_log(
			BEEMU_LOG_WARN,
			"Attempted memory address 0x%X for buffer write above maximum addressable memory address 0x%X",
			address,
			memory->memory_size - 1);
		return false;
	}
	beemu_log(BEEMU_LOG_INFO, "Writing buffered value of size %i to memory address 0x%X", size, address);
	memcpy(memory->memory + address, buffer, size);
	return true;
}

bool beemu_memory_read_buffer(BeemuMemory *memory, int address, uint8_t *buffer, int size)
{
	if (memory->memory_size <= address + size - 1)
	{
		beemu_log(
			BEEMU_LOG_WARN,
			"Attempted memory address 0x%X for buffer read above maximum addressable memory address 0x%X",
			address,
			memory->memory_size - 1);
		return false;
	}
	memcpy(buffer, memory->memory + address, size);
	return true;
}

bool beemu_memory_copy(BeemuMemory *memory, BeemuMemory *destination, int start, int dst_start, int size)
{
	if (!(memory->memory_size > (start + size)))
	{
		return false;
	}
	else if (!(destination->memory_size > (dst_start + size)))
	{
		return false;
	}
	else
	{
		memcpy(destination->memory + dst_start, memory->memory + start, size);
		return true;
	}
}

uint16_t beemu_memory_read_16(BeemuMemory *memory, uint16_t address)
{
	const uint8_t lower = beemu_memory_read(memory, address + 1);
	const uint8_t higher = beemu_memory_read(memory, address);
	beemu_log(BEEMU_LOG_INFO, "Read two bytes from 0x%X, which were 0x%X and 0x%X", address, lower, higher);
	return (((uint16_t)lower) << 8) | ((uint16_t)higher);
}

void beemu_memory_write_16(BeemuMemory *memory, uint16_t address, uint16_t value)
{
	uint8_t deconstructed[2] = {((uint8_t)(value & 0x00FF)), ((uint8_t)((value & 0xFF00) >> 8))};
	beemu_memory_write_buffer(memory, address, deconstructed, 2);
}

const uint16_t beemu_memory_block_get_size(BeemuMemoryBlock block)
{
	return block.stop - block.start;
}
