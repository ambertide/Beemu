#ifndef BEEMU_REGISTER_H
#define BEEMU_REGISTER_H
/**
 * @brief Represents 8 Bit registers.
 */
typedef enum BeemuRegister_8
{
	BEEMU_REGISTER_A,
	BEEMU_REGISTER_B,
	BEEMU_REGISTER_C,
	BEEMU_REGISTER_D,
	BEEMU_REGISTER_E,
	BEEMU_REGISTER_H,
	BEEMU_REGISTER_L,
} BeemuRegister_8;

/**
 * @brief Get one of the 8 bit registers from its letters.
 *
 * @param letter Letter that refeers to the register.
 * @return BeemuRegister_8 Register refeered or A if not valid.
 */
BeemuRegister_8 beemu_get_register_from_letter_8(const char letter);

/**
 * @brief Represents the 16 bit registers
 *
 * These include the 16 bit combination registers
 * as well as pseudo-register M and special registers
 * SP and PC.
 */
typedef enum BeemuRegister_16
{
	BEEMU_REGISTER_BC,
	BEEMU_REGISTER_DE,
	BEEMU_REGISTER_HL,
	BEEMU_REGISTER_M,
	BEEMU_REGISTER_SP,
	BEEMU_REGISTER_PC,
	BEEMU_REGISTER_AF
} BeemuRegister_16;

#endif // BEEMU_REGISTER_H
