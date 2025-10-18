from asyncore import write
from typing import Generator

from tests.resources.command_test_generators.utils import Param, emit_m1_cycle, WriteTo, Halt


def processor_state_matching(condition: str) -> str:
    """
    Get the processor state matching a given condition
    """
    is_nz_nc = 'NOT_ZERO' in condition or 'NOT_CARRY' in condition
    if is_nz_nc:
        return 'nznc'
    return 'highstack'

def processor_state_not_matching(condition: str) -> str:
    """
    Get the processor state that DOES NOT match the given jump condition
    """
    if processor_state_matching(condition) == 'nznc':
        return 'highstack'
    return 'nznc'

def emit_jump_direct(token, tests, jp_params, param: Param) -> None:
    """
    Emit test cases directly to tests for jump,
    emit both TRUE and FALSE cases for conditionals.
    """
    command_queue = [
        *emit_m1_cycle(token)
    ]

    if param.value == 'BEEMU_REGISTER_HL':
        # This is a special case for JP (HL)
        command_queue += [
            # M2/M1
            # (Contents of HL) + 1
            WriteTo.pc(0x0103),
            WriteTo.ir(0x03)
        ]
        tests.append({
            'token': token,
            'processor': 'highstack',
            'command_queue': command_queue,
            'name': f'0x{token["original_machine_code"]:06X}'
        })
        return

    lsb = param.value & 0xFF
    msb = param.value >> 8
    command_queue += [
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(lsb),
        Halt.cycle(),
        # M3
        WriteTo.pc(0x03),
        WriteTo.ir(msb),
        Halt.cycle()
    ]

    # Check the condition, if condition is falsey, then
    # M4/M1 proceeds as normal without any jump.

    # Of course since we are generating tests, we are not going to actually
    # check anything but generate both cases.

    if jp_params['condition'] != 'BEEMU_JUMP_IF_NO_CONDITION':
        truthy_processor_state = processor_state_matching(jp_params['condition'])
        falsey_processor_state = processor_state_not_matching(jp_params['condition'])
        # Append the test case where it DOESN'T jump
        tests.append({
            'token': token,
            'processor': falsey_processor_state,
            'command_queue': command_queue.copy(),
            'name': f'0x{token["original_machine_code"]:06X}NJ'
        })
        command_queue += [
            # M4
            WriteTo.pc(param.value),
            # For both highstack and nznc processor templates, lsb is the memory value
            # for a given memory addr.
            WriteTo.ir(param.value & 0xFF),
            Halt.cycle()
            # M5/M1
        ]
        tests.append({
            'token': token,
            'processor': truthy_processor_state,
            'command_queue': command_queue,
            'name': f'0x{token["original_machine_code"]:06X}J'
        })
    else:
        # For normal case, JP (a16) there is no condition so we just add the truthy
        command_queue += [
            # M4
            WriteTo.pc(param.value),
            # For both highstack and nznc processor templates, lsb is the memory value
            # for a given memory addr.
            WriteTo.ir(param.value & 0xFF),
            Halt.cycle()
            # M5/M1
        ]
        tests.append({
            'token': token,
            'processor': 'highstack',
            'command_queue': command_queue,
            'name': f'0x{token["original_machine_code"]:06X}'
        })


def emit_jump_relative(token, tests, jp_params, param: Param) -> None:
    """
    Emit a JR [cc]?, (s8) instruction
    """
    signed_payload_twos_complement = token['original_machine_code'] & 0xFF
    jr_condition = jp_params['condition']
    command_queue = [
        # M1
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(signed_payload_twos_complement),
        Halt.cycle()
    ]

    jump_dest = (0x02 + param.value + 1) % 2**16
    truthy_command_queue = [
        *command_queue,
        # M3 Spent handling ALU logic for PCH, PCL
        Halt.cycle(),
        # M4/M1
        WriteTo.pc(jump_dest),
        WriteTo.ir(jump_dest & 0xFF)
    ]

   # Emit the truthy test case
    truthy_processor = processor_state_matching(jr_condition)
    falsey_processor = processor_state_not_matching(jr_condition)

    tests.append({
        'token': token,
        'processor': truthy_processor,
        'command_queue': truthy_command_queue,
        'name': f'0x{token["original_machine_code"]:06X}J'
            if jr_condition != 'BEEMU_JUMP_IF_NO_CONDITION'
            else f'0x{token["original_machine_code"]:06X}'
    })

    if jr_condition != 'BEEMU_JUMP_IF_NO_CONDITION':
        # Also emit the no jump condition for jr cc, s8
        tests.append({
            'token': token,
            'processor': falsey_processor,
            'command_queue': command_queue,
            'name': f'0x{token["original_machine_code"]:06X}NJ'
        })

def emit_jump_part_of_call(addr: int, current_addr = 0x03) -> Generator:
    """
    Emit the jump portion off CALL/RST,
    the M values are based on CALL semantics.
    """
    # M4
    yield WriteTo.pc(0xBBFF - 1)
    yield WriteTo.ir(0xFF - 1)
    yield Halt.cycle()
    # M5
    yield WriteTo.memory(0xBBFF - 1, 0x00)
    yield WriteTo.pc(0xBBFF - 2)
    yield WriteTo.ir(0xFF - 2)
    yield Halt.cycle()
    # M6
    # Write the lower byte of the current PC to stack
    yield WriteTo.memory(0xBBFF - 2, current_addr)
    # Actually jump to the addr.
    yield WriteTo.pc(addr)
    yield WriteTo.ir(addr & 0xFF)
    yield Halt.cycle()

def emit_call(token, tests, jp_params, param: Param) -> None:
    """
    Emit call [cc?] a16 calls
    """
    command_queue = [
        # M1
        *emit_m1_cycle(token),
        # M2
        WriteTo.pc(0x02),
        WriteTo.ir(param.value & 0xFF),
        Halt.cycle(),
        # M3
        WriteTo.pc(0x03),
        WriteTo.ir(param.value >> 8),
        Halt.cycle()
    ]

    truthy_command_queue = [
        *emit_jump_part_of_call(param.value)
    ]

    truthy_processor = processor_state_matching(jp_params['condition'])
    falsey_processor = processor_state_not_matching(jp_params['condition'])

    tests.append({
        'token': token,
        'processor': truthy_processor,
        'command_queue': truthy_command_queue,
        'name': f'0x{token["original_machine_code"]:06X}'
            if jp_params['condition'] == 'BEEMU_JUMP_IF_NO_CONDITION'
            else f'0x{token["original_machine_code"]:06X}J'
    })

    if jp_params['condition'] != 'BEEMU_JUMP_IF_NO_CONDITION':
        # emit no jump test if one exists
        tests.append({
            'token': token,
            'processor': falsey_processor,
            'command_queue': command_queue,
            'name': f'0x{token["original_machine_code"]:06X}NJ'
        })


def emit_ret(token, tests, jp_params, param: Param) -> None:
    # 0xBBFF ~> FF
    # 0XBC00 ~> 00
    # RET = ...
    return_addr = 0x00FF
    truthy_processor = processor_state_matching(jp_params['condition'])
    falsey_processor = processor_state_not_matching(jp_params['condition'])
    if jp_params['condition'] != 'BEEMU_JUMP_IF_NO_CONDITION':
        falsey_command_queue = [
            # M1
            *emit_m1_cycle(token),
            Halt.cycle(),
            # M2
            # Condition check happens here.
            Halt.cycle()
        ]
        # emit no jump test if one exists
        tests.append({
            'token': token,
            'processor': falsey_processor,
            'command_queue': falsey_command_queue,
            'name': f'0x{token["original_machine_code"]:06X}NJ'
        })

    truthy_command_queue = [
        # M1
        *emit_m1_cycle(token),
        # M2
        WriteTo.register('SP', 0xBC00),
        Halt.cycle(),
        # M3
        WriteTo.register('SP', 0xBC01),
        Halt.cycle(),
        # M4
        WriteTo.pc(return_addr),
        WriteTo.ir(return_addr & 0xFF),
        *([] if not jp_params['enable_interrupts'] else [WriteTo.ime(1)]),
        Halt.cycle()
        # M5/M1
    ]

    tests.append({
        'token': token,
        'processor': truthy_processor,
        'command_queue': truthy_command_queue,
        'name': f'0x{token["original_machine_code"]:06X}'
            if jp_params['condition'] == 'BEEMU_JUMP_IF_NO_CONDITION'
            else f'0x{token["original_machine_code"]:06X}J'
    })


def emit_rst(token, tests, param: Param) -> None:
    """
    Emit a RST addr instruction
    """
    tests.append({
        'token': token,
        'processor': 'highstack',
        'command_queue': [
            # M1
            *emit_m1_cycle(token),
            # M2 -> M5/Last,
            *emit_jump_part_of_call(param.value, 0x01)
        ]
    })

def emit_jump_tests(tokens) -> list[dict]:
    tests = []
    for token in tokens:
        emitted_token = token['token']
        jp_params = emitted_token['params']['jump_params']
        jp_type = jp_params['type']
        is_relative = jp_params['is_relative']
        is_conditional = jp_params['is_conditional']
        param = Param.from_dict(jp_params['param'])
        match (jp_type, is_relative, is_conditional):
            case ('BEEMU_JUMP_TYPE_JUMP', False, _):
                emit_jump_direct(emitted_token, tests, jp_params, param)
                continue
            case ('BEEMU_JUMP_TYPE_JUMP', True, _):
                emit_jump_relative(emitted_token, tests, jp_params, param)
            case ('BEEMU_JUMP_TYPE_CALL', _, _):
                emit_call(emitted_token, tests, jp_params, param)
            case ('BEEMU_JUMP_TYPE_RET', _, _):
                emit_ret(emitted_token, tests, jp_params, param)
            case ('BEEMU_JUMP_TYPE_RST', _, _):
                emit_rst(emitted_token, tests, param)
            case _:
                print(token['instruction'])
                continue
    return tests