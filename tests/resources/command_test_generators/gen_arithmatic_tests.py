from json import load, dumps
from collections.abc import Callable

from tests.resources.command_test_generators.utils import get_tokens_in_range, WriteTo, Halt
from dataclasses import dataclass
from itertools import chain, islice
from json import dump

from tests.resources.token_test_generators.utils import get_opcode, sort_instructions

# btw this is all the tokens in the 8 bit arithmatic mainline and preline.
tokens = get_tokens_in_range(chain(range(0x80, 0xC0), range(0x04, 0x3D, 8), range(0x05, 0x3F, 8), range(0x03, 0x3C, 8)))

# The below values are hardcoded for the DEFAULT processor preset for testing
# S and P are not real 8 bit registers but they are here for the sake of brevity
register_index = ['A', 'B', 'C', 'D', 'E', 'H', 'L', 'S', 'P']
register_values = [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02, 0x00, 0xFF]

val_functions = {
    "ADD": lambda a, b: (a + b) % 256,
    # In default processor the C flag is set to 1
    "ADC": lambda a, b: (a + b + 1) % 256,
    "SUB": lambda a, b: (a - b) % 256,
    # In default processor the C flag is set to 1
    "SBC": lambda a, b: (a - b - 1) % 256,
    "AND": lambda a, b: a & b,
    "OR": lambda a, b: a | b,
    "CP": lambda a, _: a,
    "XOR": lambda a, b: a ^ b,
}

def emit_m1_cycle(token: dict) -> list[dict]:
    return [
        WriteTo.address_bus(0x01),
        WriteTo.pc(0x01),
        WriteTo.data_bus(get_opcode(token['original_machine_code'])),
        WriteTo.ir(get_opcode(token['original_machine_code'])),
        Halt.cycle(),
    ]


@dataclass()
class FlagStates:
    z: int
    n: int
    h: int
    c: int

    def generate_flag_write_commands(self):
        flags = [self.z, self.n, self.h, self.c]
        for flag_name, flag_value in zip(['Z', 'N', 'H', 'C'], flags):
            if flag_value == -1:
                continue

            if flag_value in [True, False]:
                # Normalize flag values.
                flag_value = 1 if flag_value else 0

            yield WriteTo.flag(flag_name, flag_value)

flag_functions = {
    "ADD": lambda a, b: FlagStates(
        z=((a + b) % 256) == 0,
        n=0,
        # Mask the lower bits and add them.
        h=((a & 0x0F) + (b & 0x0F)) & 0x10 == 0x10,
        c=(a + b) > 0xFF
    ),
    "ADC": lambda a, b: FlagStates(
        z=((a + b + 1) % 256) == 0,
        n=0,
        h=((a & 0x0F) + (b & 0x0F) + 1) & 0x10 == 0x10,
        c=(a + b + 1) > 0xFF),
    "SUB": lambda a, b: FlagStates(
        z=((a - b) % 256) == 0,
        n=1,
        # Likewise for borrows this occurs for sub 0 underflow
        h=(((a & 0xF) - (b & 0x0F)) % 256) & 0x10 == 0x10,
        c=(a - b) < 0x00),
    "SBC": lambda a, b: FlagStates(
        z=((a - b - 1) % 256) == 0,
        n=1,
        h=(((a & 0xF) - (b & 0x0F)) - 1) & 0x10 == 0x10,
        c=(a - b - 1) < 0x00),
    "AND": lambda a, b: FlagStates(
        z=a & b == 0,
        n=0,
        h=1,
        c=0),
    "XOR": lambda a, b: FlagStates(
        z=a ^ b == 0,
        n=0,
        h=0,
        c=0),
    "OR": lambda a, b: FlagStates(
        z=a | b == 0,
        n=0,
        h=0,
        c=0),
    "CP": lambda a, b: FlagStates(
        z=a - b == 0,
        n=1,
        h=((a & 0xF) - (b & 0x0F)) & 0x10 == 0x10,
        c=(a - b) < 0x00),
}

tests = []

def emit_8_bit_mainline(token: dict, tests: list, val_func: Callable, flag_func: Callable) -> None:
    """
    Emit a 8 bit arithmatic mainline instruction.
    """
    # First one is always the A register, which is always 0x0A
    # second one depends on the  token.
    values = [0x0A]
    second_register = token["params"]["arithmatic_params"]["source_or_second"]['value']['register_8'].replace('BEEMU_REGISTER_', '') if token["params"]["arithmatic_params"]["source_or_second"]['type'] == 'BEEMU_PARAM_TYPE_REGISTER_8' else 'HL'

    # if hl this is always a pointer and since HL is always 0x0102, [0x0102] is...
    second_value = 0x02 if second_register == 'HL' else register_values[register_index.index(second_register)]

    values.append(second_value)

    # Now calculate the results
    operation_result = val_func(*values)
    flag_values: FlagStates = flag_func(*values)


    if second_register == 'HL':
        tests.append({
            "token": token,
            "processor": "default",
            "command_queue": [
                # M1 Begins
                *emit_m1_cycle(token),
                # M2 begins
                WriteTo.address_bus(0x0102),
                WriteTo.data_bus(0x02),
                # M2 ends M3/M1 begins
                Halt.cycle(),
                *([WriteTo.register('A', operation_result)] if operation != 'CP' else []),
                # Add the flag writes
                *flag_values.generate_flag_write_commands(),
                WriteTo.address_bus(0x01),
                WriteTo.data_bus(get_opcode(token['original_machine_code']))
            ]
        })
    else:
        # Otherwise we just from register to register.
        tests.append({
            "token": token,
            "processor": "default",
            "command_queue": [
                # M1 begins
                *emit_m1_cycle(token),
                # M2/M1 begins
                *([WriteTo.register('A', operation_result)] if operation != 'CP' else []),
                *flag_values.generate_flag_write_commands()
            ]
        })

def emit_8_bit_preline(token: dict, tests: list, val_func: Callable, flag_func: Callable) -> None:
    """
    Emit a preline arithmatic instruction in the preline segment.
    """
    # This is the pre-mainline
    inc_dec_register = token["params"]["arithmatic_params"]["dest_or_first"]['value']['register_8'].replace('BEEMU_REGISTER_', '') if token["params"]["arithmatic_params"]["dest_or_first"]['type'] == 'BEEMU_PARAM_TYPE_REGISTER_8' else 'HL'
    # just get the value to be incremented or decremented
    first_value = 0x02 if inc_dec_register == 'HL' else register_values[register_index.index(inc_dec_register)]
    # And then the second is always 0x01 for INC or DEC
    values = [first_value, 0x01]

    operation_result = val_func(*values)
    flag_values = flag_func(*values)

    # Rather importantly, the Carry flag is NOT set for INC/DEC

    if inc_dec_register == 'HL':
        tests.append({
            "token": token,
            "processor": "default",
            "command_queue": [
                # M1 begins
                *emit_m1_cycle(token),
                # M2/M1 Begins
                WriteTo.address_bus(0x0102),
                WriteTo.data_bus(0x02),
                Halt.cycle(),
                # M3 begins
                WriteTo.data_bus(operation_result),
                WriteTo.memory(0x0102, operation_result),
                *islice(flag_values.generate_flag_write_commands(), 3),
                Halt.cycle(),
                # M4/M1 begins
                WriteTo.address_bus(0x01),
                WriteTo.data_bus(get_opcode(token['original_machine_code']))
            ]
        })
    else:
        tests.append({
            "token": token,
            "processor": "default",
            "command_queue": [
                # M1 Begins
                *emit_m1_cycle(token),
                # M2/M1 Begins
                WriteTo.register(inc_dec_register, operation_result),
                # Toss away the C write because we do not set the Cy.
                *islice(flag_values.generate_flag_write_commands(), 3)
            ]
        })

def emit_16_bit_inc_dec(token: dict, test: list, val_func: Callable, flag_func: Callable):
    """
    Emit a preline arithmatic instruction that is a 16 bit inc dec
    """
    inc_dec_register = token["params"]["arithmatic_params"]["dest_or_first"]['value']['register_16'].replace('BEEMU_REGISTER_', '')
    # just get the value to be incremented or decremented
    eight_bit_parts = [*inc_dec_register]
    first_value = 0
    for eight_bit_register in eight_bit_parts:
        # A 16 bit register is just the concat of 2 8 bit registers.
        val = register_values[register_index.index(eight_bit_register)]
        first_value <<= 4
        first_value |= val
    # And then the second is always 0x01 for INC or DEC
    values = [first_value, 0x01]

    operation_result = val_func(*values)
    flag_values = flag_func(*values)

    # Rather importantly, the Carry flag is NOT set for INC/DEC

    print(inc_dec_register)
    tests.append({
        "token": token,
        "processor": "default",
        "command_queue": [
            # M1 Begins
            *emit_m1_cycle(token),
            # M2/M1 Begins
            # temporarily uses PC as a ad-hoc 16 bit data bus.
            WriteTo.address_bus(first_value),
            WriteTo.register(inc_dec_register, first_value + 1),
            Halt.cycle(),
            # Restore PC
            WriteTo.address_bus(0x01)
            # No flags are modified for IDU operations.
        ]
    })

def emit_16_bit_preline(token: dict, test: list, *args, **kwargs):
   match token['original_machine_code'] & 0x0F:
       case 0x09:
           ...
       case 0x03:
           emit_16_bit_inc_dec(token, test, *args, **kwargs)
       case 0x0B:
           emit_16_bit_inc_dec(token, test, *args, **kwargs)

for token in tokens:
    token = token['token']
    operation = token["params"]['arithmatic_params']['operation'].replace('BEEMU_OP_', '')
    val_func = val_functions[operation]
    flag_func = flag_functions[operation]

    is_preline = token["original_machine_code"] < 0xC0
    is_mainline = 0xC0 > token["original_machine_code"] >= 0x80
    is_8_bit = (token["params"]["arithmatic_params"]["dest_or_first"]['type'] == 'BEEMU_PARAM_TYPE_REGISTER_8'
                or token["params"]["arithmatic_params"]["dest_or_first"]['pointer'])
    if is_mainline:
        emit_8_bit_mainline(token, tests, val_func, flag_func)
    elif is_preline:
        if is_8_bit:
            emit_8_bit_preline(token, tests, val_func, flag_func)
        else: # is_16_bit
            emit_16_bit_preline(token, tests, val_func, flag_func)

sort_instructions(tests, lambda test: test["token"]["original_machine_code"])

if __name__ == '__main__':
    with open('../command_tests.json', 'w') as f:
        dump({ 'commands': tests }, f, indent='\t')