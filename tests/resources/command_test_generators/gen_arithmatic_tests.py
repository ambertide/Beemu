from json import load, dumps

from tests.resources.command_test_generators.utils import get_tokens_in_range, WriteTo, Halt
from dataclasses import dataclass
from json import dump

# btw this is all the tokens in the arithmatic mainline.
tokens = get_tokens_in_range(range(0x80, 0xC0))

# The below values are hardcoded for the DEFAULT processor preset for testing
register_index = ['A', 'B', 'C', 'D', 'E', 'H', 'L']
register_values = [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02]

val_functions = {
    "ADD": lambda a, b: (a + b) % 256,
    # In default processor the C flag is set to 1
    "ADC": lambda a, b: (a + b + 1) % 256,
    "SUB": lambda a, b: (a - b) % 256,
    # In default processor the C flag is set to 1
    "SBC": lambda a, b: (a - b) % 256,
    "AND": lambda a, b: a & b,
    "OR": lambda a, b: a | b,
    "CP": lambda a, _: a,
    "XOR": lambda a, b: a ^ b,
}


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
        h=((a + b) % 256) > 0x0F,
        c=(a + b) > 0xFF),
    "ADC": lambda a, b: FlagStates(
        z=((a + b + 1) % 256) == 0,
        n=0,
        h=((a + b + 1) % 256) > 0x0F,
        c=(a + b + 1) > 0xFF),
    "SUB": lambda a, b: FlagStates(
        z=((a - b) % 256) == 0,
        n=1,
        h=((a - b) % 256) > 0x0F,
        c=(a - b) < 0x00),
    "SBC": lambda a, b: FlagStates(
        z=((a - b - 1) % 256) == 0,
        n=1,
        h=((a - b - 1) % 256) > 0x0F,
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
        h=((a - b - 1) % 256) > 0x0F,
        c=(a - b - 1) < 0x00),
}

tests = []

for token in tokens:
    token = token['token']
    operation = token["params"]['arithmatic_params']['operation'].replace('BEEMU_OP_', '')
    val_func = val_functions[operation]
    flag_func = flag_functions[operation]
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
                WriteTo.address_bus(0x0102),
                WriteTo.data_bus(0x02),
                # M2 ends M3/M1 begins
                Halt.cycle(),
                WriteTo.register('A', operation_result),
                # Add the flag writes
                *flag_values.generate_flag_write_commands()
            ]
        })

if __name__ == '__main__':
    with open('../command_tests.json', 'w') as f:
        dump({ 'commands': tests }, f, indent='\t')