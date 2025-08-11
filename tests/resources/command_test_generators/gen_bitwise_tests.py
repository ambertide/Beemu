# btw this is all the tokens in the 8 bit arithmatic mainline and preline.
from tests.resources.command_test_generators.utils import get_tokens_of_type, Halt, WriteTo, emit_m1_cycle

# The below values are hardcoded for the DEFAULT processor preset for testing
# S and P are not real 8 bit registers but they are here for the sake of brevity
register_index = ['A', 'B', 'C', 'D', 'E', 'H', 'L', 'S', 'P']
register_values = [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02, 0x00, 0xFF]

operation_map = {
    'BIT': lambda byte, bit: (byte & 2**bit) >> bit,
    'RES': lambda byte, bit: (byte & ~(2**bit)),
    'SET': lambda byte, bit: (byte | (2**bit))
}

def emit_m2_cycle(token):
    return [
        # Increment PC to read CBXX opcode
        WriteTo.pc(0x02),
        # Write actually op to IR.
        WriteTo.ir(token['original_machine_code'] & 0xFF),
        Halt.cycle(),
    ]

def emit_bitwise_tests(tokens) -> list[dict]:
    tests = []
    for token in tokens:
        params = token['token']['params']['bitwise_params']
        operation = params['operation'].replace('BEEMU_BIT_OP_', '')
        impacts_bit = params['bit_number']
        command_queue = [
            # M1 ends
            *emit_m1_cycle(token['token'], 0xCB),
            # M2 ends with extra read for CBXX operation
            *emit_m2_cycle(token['token'])
        ]
        if params['target']['type'] == 'BEEMU_PARAM_TYPE_REGISTER_8':
            register =  params['target']['value']['register_8'].replace('BEEMU_REGISTER_', '')
            value_ = register_values[register_index.index(register)]
            result = operation_map[operation](value_, impacts_bit)
            if operation != 'BIT':
                command_queue.extend([
                    # M3/M1
                    WriteTo.register(register, result)
                ])
            else:
                command_queue.extend([
                    # M3/M1 is the actual OP
                    # But bit only sets the flags.
                    WriteTo.flag('Z', result),
                    WriteTo.flag('N', 0),
                    WriteTo.flag('H', 1)
                ])
            # And then if the operation is a BIT
            # then we actually
        else:
            # Register is HL,
            # And in default preset, HL is, 0x102
            # And, in the default preset, 0x102 mem addr holds 0x02
            mem_addr = 0x0102
            value_ = 0x02
            result = operation_map[operation](value_, impacts_bit)
            if operation != 'BIT':
                command_queue.extend([
                    # We have to first read the data,
                    # this does nto emit commands but does halt.
                    # M3
                    Halt.cycle(),
                    # M4
                    WriteTo.memory(mem_addr, result),
                    Halt.cycle()
                    # M5/M1 is used to restore the busses.
                    # or flush the memory, unsure.
                ])
            else:
                command_queue.extend([
                    # We have to first read the data,
                    # this does nto emit commands but does halt.
                    # M3
                    Halt.cycle(),
                    # M4/M1 is used to just write to flags
                    # as we do not write to memory.
                    WriteTo.flag('Z', result),
                    WriteTo.flag('N', 0),
                    WriteTo.flag('H', 1)
                ])
        tests.append({
            "token": token['token'],
            "processor": "default",
            "command_queue": command_queue
        })
    return tests
