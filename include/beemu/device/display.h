#ifndef BEEMU_DEVICE_Display_H
#define BEEMU_DEVICE_Display_H
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum BeemuLCDControlOperation
	{
		BEEMU_LCDC_OPERATION_STOP,
		BEEMU_LCDC_OPERATION_CONTROL
	} BeemuLCDControlOperation;

	typedef enum BeemuSpriteSize
	{
		BEEMU_SPRITE_SIZE_8_TO_8,
		BEEMU_SPRITE_SIZE_8_TO_16
	} BeemuSpriteSize;

	typedef struct BeemuDisplayLCDC
	{
		BeemuLCDControlOperation operation;
		BeemuMemoryBlock window_tile_map_display_select;
		bool window_display_on;
		BeemuMemoryBlock bg_window_tile_data_select;
		BeemuMemoryBlock bg_tile_map_display_select;
		BeemuSpriteSize sprite_size;
		bool sprite_display_on;
		bool bg_and_window_display_on;
	} BeemuDisplayLCDC;

	typedef struct BeemuDisplayState
	{
		uint8_t scroll_y;
		uint8_t scroll_x;
		uint8_t lcdc_y;
		uint8_t lcdc_y_compare;
		uint8_t dma_address;
		BeemuDisplayLCDC lcdc_register;
	} BeemuDisplayState;

	typedef struct BeemuDisplay
	{
		BeemuMemory *memory;
	} BeemuDisplay;

#ifdef __cplusplus
}
#endif

#endif // BEEMU_DEVICE_Display_H