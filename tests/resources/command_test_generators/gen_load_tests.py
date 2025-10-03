# The below values are hardcoded for the DEFAULT processor preset for testing
# S and P are not real 8 bit registers but they are here for the sake of brevity
from tests.resources.command_test_generators.gen_arithmatic_tests import flag_functions
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
    'DE': 0x0D0E,
    'AF': 0x0AF0
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
    direct_lsb = src.value & 0xFF
    direct_msb = (src.value & 0xFF00) >> 8
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
    ]

def emit_sp_load(token, dst: Param, src: Param) -> list[dict]:
    # LD (a16), SP
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir((dst.value & 0xFF)),
        Halt.cycle(),
        # M3
        WriteTo.pc(0x03),
        WriteTo.ir((dst.value & 0xFF00) >> 8),
        Halt.cycle(),
        # M4
        # This is the SP value for highstack
        WriteTo.memory(dst.value, 0xFF),
        Halt.cycle(),
        # M5
        WriteTo.memory(dst.value + 1, 0xBB),
        Halt.cycle(),
        # M6/M1
    ]

def emit_pop(token, dst: Param, src: Param, pst_ld_op: str) -> list[dict]:
    """
    Pop R16 tests
    """
    high_stack_sp = 0xBBFF
    high_stack_sp_deref = 0xFF
    high_stack_sp_deref_plus_one = 0x00
    return [
        *emit_m1_cycle(token),
        # Increment the SP twice to capture the stack pointer value.
        # M2
        WriteTo.register('SP', high_stack_sp + 1),
        Halt.cycle(),
        # M3
        WriteTo.register('SP', high_stack_sp + 2),
        Halt.cycle(),
        # M4/M1
        WriteTo.register(dst.register, (high_stack_sp_deref_plus_one << 8) | high_stack_sp_deref)
    ]
def emit_push(token, dst: Param, src: Param, pst_ld_op: str) -> list[dict]:
    """
    Tests for PUSH R16
    """
    reg_value = mem_addresses[src.register]
    high_stack_sp = 0xBBFF
    msb = (reg_value & 0xFF00) >> 8
    lsb = reg_value & 0xFF
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.register('SP', high_stack_sp - 1),
        Halt.cycle(),
        # M3
        WriteTo.memory(high_stack_sp - 1, msb),
        WriteTo.register('SP', high_stack_sp - 2),
        Halt.cycle(),
        # M4
        WriteTo.memory(high_stack_sp - 2, lsb),
        Halt.cycle()
        # M5/M1
    ]

def emit_d8_ptr_a(token, dst: Param, src: Param) -> list[dict]:
    """
    LDH (a8), A
    """
    return [
        *emit_m1_cycle(token),
        # M2
        # It is actually unclear to me if IR is actually overwritten
        # but it makes sense to me if PC is done.
        WriteTo.pc(0x02),
        WriteTo.ir(dst.value),
        Halt.cycle(),
        # M3
        WriteTo.memory(0xFF00 + dst.value, 0x0A),
        Halt.cycle(),
        # M4/M1
    ]

def emit_a_d8_ptr(token, dst: Param, src: Param) -> list[dict]:
    """
    LDH A, (a8)
    """
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(src.value),
        Halt.cycle(),
        # M3
        # Spent fetching memory
        Halt.cycle(),
        # M4/M1
        # [0xFFBC] = 0xBC
        WriteTo.register(dst.register, 0xBC)
    ]

def emit_d16_ptr_a(token, dst: Param, src: Param) -> list[dict]:
    """
    LD (a16), A
    """
    return [
        *emit_m1_cycle(token),
        # M2
        # It is actually unclear to me if IR is actually overwritten
        # but it makes sense to me if PC is done.
        WriteTo.pc(0x02),
        WriteTo.ir(dst.value & 0xFF),
        Halt.cycle(),
        # M3
        WriteTo.pc(0x03),
        WriteTo.ir((dst.value & 0xFF00) >> 8),
        Halt.cycle(),
        # M4
        WriteTo.memory(dst.value, 0x0A),
        Halt.cycle(),
        # M5/M1
    ]

def emit_a_d16_ptr(token, dst: Param, src: Param) -> list[dict]:
    """
    LD A, (a16)
    """
    return [
        *emit_m1_cycle(token),
        # M2
        # It is actually unclear to me if IR is actually overwritten
        # but it makes sense to me if PC is done.
        WriteTo.pc(0x02),
        WriteTo.ir(src.value & 0xFF),
        Halt.cycle(),
        # M3
        WriteTo.pc(0x03),
        WriteTo.ir((src.value & 0xFF00) >> 8),
        Halt.cycle(),
        # M4
        Halt.cycle(),
        # M5/M1
        WriteTo.register(dst.register, 0xBC),
    ]

def emit_ldh_c_deref_to_a(token, dst: Param, src: Param) -> list[dict]:
    # 0XFF0C
    mem_addr = 0xFF00 + register_val_dict[src.register]

    return [
        *emit_m1_cycle(token),
        # M2
        # spent fetching from memory
        Halt.cycle(),
        # M3/M1
        WriteTo.register('A', 0x0C)
    ]

def emit_ldh_a_to_c_deref(token, dst: Param, src: Param) -> list[dict]:
    """
    LDH (C), A
    """
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.memory(0xFF0C, 0x0A),
        Halt.cycle(),
        # M3/M1
    ]

def emit_sp_hl(token, pst_ld_op, pst_ld_param: Param) -> list[dict]:
    """
    LD SP, HL & LD HL, SP + s8
    """
    if pst_ld_op == 'BEEMU_POST_LOAD_SIGNED_PAYLOAD_SUM':
        ## According to https://gekkio.fi/files/gb-docs/gbctr.pdf
        # The flag calculations occur in M3 wrt to the least significant
        # byte of the SP and it treats as an addition.
        flag_ops = flag_functions['ADD']
        absolute_value = pst_ld_param.value * -1 if pst_ld_param.value < 0 else pst_ld_param.value
        flag_vals = flag_ops(0xff, 0xf0, 256)
        genvalue = (0xbbff + pst_ld_param.value) % 2**16
        l_value = genvalue & 0xFF
        h_value = genvalue >> 8
        return [
            *emit_m1_cycle(token),
            # M2
            # Spent fetching the s8.
            WriteTo.pc(0x02),
            WriteTo.ir(token['original_machine_code'] & 0xFF),
            Halt.cycle(),
            # M3
            WriteTo.register('L', l_value),
            *flag_vals.generate_flag_write_commands(),
            Halt.cycle(),
            # M4 / M1
            WriteTo.register('H', h_value + int(flag_vals.c))
        ]
    return [
        *emit_m1_cycle(token),
        # M2
        WriteTo.register('SP', mem_addresses['HL']),
        Halt.cycle()
        # M3
    ]

def emit_load_tests(tokens) -> list[dict]:
    tests = []
    for token in tokens:
        emitted_token = token['token']
        ld_params = emitted_token['params']['load_params']
        src = Param.from_dict(ld_params['source'])
        dst = Param.from_dict(ld_params['dest'])
        pst_ld_op = ld_params['postLoadOperation']
        processor = 'default'
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
            case (True, 'BEEMU_PARAM_TYPE_UINT16', False, 'BEEMU_PARAM_TYPE_REGISTER_16'):
                processor = 'highstack'
                command_queue = emit_sp_load(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_16', True, 'BEEMU_PARAM_TYPE_REGISTER_16'):
                processor = 'highstack'
                command_queue = emit_pop(emitted_token, dst, src, pst_ld_op)
            case (True, 'BEEMU_PARAM_TYPE_REGISTER_16', False, 'BEEMU_PARAM_TYPE_REGISTER_16'):
                processor = 'highstack'
                command_queue = emit_push(emitted_token, dst, src, pst_ld_op)
            case (True, 'BEEMU_PARAM_TYPE_UINT_8', False, 'BEEMU_PARAM_TYPE_REGISTER_8'):
                command_queue = emit_d8_ptr_a(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_8', True, 'BEEMU_PARAM_TYPE_UINT_8'):
                command_queue = emit_a_d8_ptr(emitted_token, dst, src)
            case (True, 'BEEMU_PARAM_TYPE_UINT16', False, 'BEEMU_PARAM_TYPE_REGISTER_8'):
                command_queue = emit_d16_ptr_a(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_8', True, 'BEEMU_PARAM_TYPE_UINT16'):
                command_queue = emit_a_d16_ptr(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_8', True, 'BEEMU_PARAM_TYPE_REGISTER_8'):
                command_queue = emit_ldh_c_deref_to_a(emitted_token, dst, src)
            case (True, 'BEEMU_PARAM_TYPE_REGISTER_8', False, 'BEEMU_PARAM_TYPE_REGISTER_8'):
                command_queue = emit_ldh_a_to_c_deref(emitted_token, dst, src)
            case (False, 'BEEMU_PARAM_TYPE_REGISTER_16', False, 'BEEMU_PARAM_TYPE_REGISTER_16'):
                processor = 'highstack'
                pst_ld_param = Param('BEEMU_REGISTER_8', False, 'BEEMU_REGISTER_A')
                if pst_ld_op == 'BEEMU_POST_LOAD_SIGNED_PAYLOAD_SUM':
                    pst_ld_param = Param.from_dict(ld_params['auxPostLoadParameter'])
                command_queue = emit_sp_hl(emitted_token, pst_ld_op, pst_ld_param)
            case _:
                print(token['instruction'])
                continue
        tests.append({
            'token': emitted_token,
            'processor': processor,
            'command_queue': command_queue
        })
    return tests
