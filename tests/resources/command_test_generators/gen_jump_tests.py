from asyncore import write

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
            case _:
                print(token['instruction'])
                continue
    return tests