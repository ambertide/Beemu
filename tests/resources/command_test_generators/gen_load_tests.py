# The below values are hardcoded for the DEFAULT processor preset for testing
# S and P are not real 8 bit registers but they are here for the sake of brevity
from tests.resources.command_test_generators.utils import Param, emit_m1_cycle, WriteTo, Halt
from re import match

register_index = ['A', 'B', 'C', 'D', 'E', 'H', 'L', 'S', 'P']
register_values = [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02, 0x00, 0xFF]
register_val_dict = { k: v for k, v in zip(register_index, register_values)}

mem_addr_values = {
    'HL': 0x02,
    'BC': 0x0C,
    'DE': 0x0E
}

mem_addresses = {
    'HL': 0x0102,
    'BC': 0x0B0C,
    'DE': 0x0D0E
}

def emit_post_load(reg: Param, post_load_param: str) -> list[dict]:
    """
    Emit a increment or decrement post load operation
    """
    if post_load_param in ('BEEMU_POST_LOAD_NOP', 'BEEMU_POST_LOAD_SIGNED_PAYLOAD_SUM'):
        return []
    reg_value = mem_addresses[reg.register]
    matches = match(r"BEEMU_POST_LOAD_(DECREMENT|INCREMENT)_INDIRECT_\w+", post_load_param)
    if not matches:
        return []
    operation = matches.group(1)
    match operation:
        case 'INCREMENT':
            return [
                WriteTo.register(reg.register, (reg_value + 1) % 2**16)
            ]
        case 'DECREMENT':
            return [
                WriteTo.register(reg.register, (reg_value - 1) % 2**16)
            ]
    return []

def emit_eight_bit_inter_register_load(token: dict, dst: Param, src: Param) -> list[dict]:
    """
    Emit an eight bit ld op between two registers
    """
    register_value = register_val_dict[src.register]

    return [
        *emit_m1_cycle(token),
        WriteTo.register(dst.register, register_value)
    ]

def emit_eight_bit_hl_deref_load(token: dict, dst: Param, src: Param, post_load_param) -> list[dict]:
    """
    Essentially, LD r, [HL] or [BC] or [DE]
    """
    return [
        *emit_m1_cycle(token),
        # Extra halt for this cycle is spent fetching [0x0102]
        *emit_post_load(src, post_load_param),
        Halt.cycle(),
        # Then we must write to the destination the 0x02, which is
        # the mem value at 0x0102.
        WriteTo.register(dst.register, mem_addr_values[src.register])
    ]

def emit_eight_bit_load_to_derefed_memory(token, dst: Param, src: Param, post_load_param: str) -> list[dict]:
    return [
        *emit_m1_cycle(token),
        WriteTo.memory(mem_addresses[dst.register], register_val_dict[src.register]),
        *emit_post_load(dst, post_load_param),
        Halt.cycle()
        # Mem writes often halt the cycle, though I am not sure why.
        # Next cycle is M3/M1.
    ]

def emit_sixteen_bit_load_direct(token, dst: Param, src: Param) -> list[dict]:
    """
    This adds tests for LD rr, d16 cases.
    """
    direct_msb = src.value & 0xFF
    direct_lsb = (src.value & 0xFF00) >> 8
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(direct_lsb),
        Halt.cycle(),
        # M3
        WriteTo.pc(0x03),
        WriteTo.ir(direct_msb),
        Halt.cycle(),
        # M4/M1, by this point the value have been read from memory.
        WriteTo.register(dst.register, src.value)
    ]

def emit_eight_bit_load_direct(token, dst: Param, src: Param) -> list[dict]:
    """
    This adds tests for LD r, d8 cases
    """
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(src.value),
        Halt.cycle(),
        # M3/M1
        WriteTo.register(dst.register, src.value)
    ]

def emit_eight_bit_dereffed_write_to_hl(token, dst: Param, src: Param) -> list[dict]:
    """
    This adds test for LD [HL], d8 case.
    """
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(src.value),
        Halt.cycle(),
        # M3
        WriteTo.memory(0x0102, src.value),
        Halt.cycle(),
        # M4/M1, extra halt for write op.
        Halt.cycle()
    ]

def emit_load_tests(tokens) -> list[dict]:
    tests = []
    for token in tokens:
        emitted_token = token['token']
        ld_params = emitted_token['params']['load_params']
        src = Param.from_dict(ld_params['source'])
        dst = Param.from_dict(ld_params['dest'])
        pst_ld_op = ld_params['postLoadOperation']
        match (dst.pointer, dst.type, src.pointer, src.type):
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_8', False, 'BEEMU_PARAM_TYPE_REGISTER_8'):
                command_queue = emit_eight_bit_inter_register_load(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_8', True, 'BEEMU_PARAM_TYPE_REGISTER_16'):
                command_queue = emit_eight_bit_hl_deref_load(emitted_token, dst, src, pst_ld_op)
            case (True, 'BEEMU_PARAM_TYPE_REGISTER_16', False, 'BEEMU_PARAM_TYPE_REGISTER_8'):
                command_queue = emit_eight_bit_load_to_derefed_memory(emitted_token, dst, src, pst_ld_op)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_16', False, 'BEEMU_PARAM_TYPE_UINT16'):
                command_queue = emit_sixteen_bit_load_direct(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_8', False, 'BEEMU_PARAM_TYPE_UINT_8'):
                command_queue = emit_eight_bit_load_direct(emitted_token, dst, src)
            case (True, 'BEEMU_PARAM_TYPE_REGISTER_16', False, 'BEEMU_PARAM_TYPE_UINT_8'):
                command_queue = emit_eight_bit_dereffed_write_to_hl(emitted_token, dst, src)
            case _:
                print(token['instruction'])
                continue
        tests.append({
            'token': emitted_token,
            'processor': 'default',
            'command_queue': command_queue
        })
    return tests
