"""
Print out the emitted bytecodes for a specific instruction
by the `beemu_parser_parse` function using `lldb`
"""
from json import load
from typing import Generator

import lldb
import os
from sys import argv
from subprocess import Popen

opcode = argv[1]
should_compile = argv[2] == 'compile' if len(argv) > 2 else False

print(opcode)

from lldb import SBValue

target_map = {
    "BEEMU_WRITE_TARGET_REGISTER_16": "register_16",
    "BEEMU_WRITE_TARGET_REGISTER_8": "register_8" ,
    "BEEMU_WRITE_TARGET_MEMORY_ADDRESS": "mem_addr",
    "BEEMU_WRITE_TARGET_FLAG": "flag",
    "BEEMU_WRITE_TARGET_INTERNAL": "internal_target"
}

value_map = {
    'true': 'double_value',
    'false': 'byte_value'
}

def parse_expected_node() -> Generator[str, None, None]:
    with open('tests/resources/command_tests.json') as f:
        all_instrs: list = load(f)['commands']
    instr = next(inst for inst in all_instrs if inst['token']['original_machine_code'] == int(opcode, base=16))
    commands = instr['command_queue']
    for command in commands:
        if command['type'] == 'BEEMU_COMMAND_WRITE':
            command_target = [*command['write']['target']['target'].values()][0]
            command_values = [*command['write']['value']['value'].values()][0]
            yield f'WRITE {command_target} {command_values}'
        elif command['type'] == 'BEEMU_COMMAND_HALT':
            yield 'HALT'
        else:
            yield 'UNSUPPORTED'



def parse_node(instr: lldb.SBValue) -> str:
    type_: SBValue = instr.GetChildMemberWithName('type').GetValue()
    if type_ == 'BEEMU_COMMAND_WRITE':
        write_info = instr.GetChildMemberWithName('write')
        write_target_info = write_info.GetChildMemberWithName('target')
        write_type = write_target_info.GetChildMemberWithName('type').GetValue()
        write_target_field = target_map.get(write_type)
        write_target = write_target_info.GetChildMemberWithName('target').GetChildMemberWithName(write_target_field).GetValue()
        write_value_info = write_info.GetChildMemberWithName('value')
        write_value_type = write_value_info.GetChildMemberWithName('is_16').GetValue()
        write_value_field = value_map.get(write_value_type)
        write_value: str = write_value_info.GetChildMemberWithName('value').GetChildMemberWithName(write_value_field).GetValue()
        if write_value.startswith("'") and len(write_value.replace("'", '')) == 1:
            write_value: int = ord(write_value.replace("'", ''))
        return f'WRITE {write_target} {write_value}'
    elif type_ == 'BEEMU_COMMAND_HALT':
        return 'HALT'
    else:
        return 'UNSUPPORTED'


def print_on_centre(ls: str, rs: str) -> None:
    left_str_len = len(ls)
    right_str_len = len(rs)
    print(f"{ls}{' ' * (80 - left_str_len)}{rs}")

def compile_debug():
    proc = Popen('cmake -DCMAKE_BUILD_TYPE=Debug .. && make', shell=True, cwd=os.getcwd() + '/build')
    proc.wait()

if __name__ == '__main__':
    if should_compile:
        compile_debug()
    # Set the path to the executable to debug
    exe = "./build/tests/tests"
    gtest_filter = f'--gtest_filter=BeemuParserTests/BeemuParserParameterizedTestFixture.TokenParsedCorrectly/{opcode}'
    # Create a new debugger instance
    debugger = lldb.SBDebugger.Create()

    # When we step or continue, don't return from the function until the process
    # stops. Otherwise we would have to handle the process events ourselves which, while doable is
    # a little tricky.  We do this by setting the async mode to false.
    debugger.SetAsync(False)

    # Create a target from a file and arch
    print("Creating a target for '%s'" % exe)

    target = debugger.CreateTargetWithFileAndArch(exe, lldb.LLDB_ARCH_DEFAULT)

    if target:
        # If the target is valid set a breakpoint at main
        main_bp = target.BreakpointCreateByName(
            "beemu_parser_parse", target.GetExecutable().GetFilename()
        )

        print(main_bp)

        # Launch the process. Since we specified synchronous mode, we won't return
        # from this function until we hit the breakpoint at main
        process = target.LaunchSimple([gtest_filter], None, os.getcwd())

        # Make sure the launch went ok
        if process:
            # Print some simple process info
            stdout = process.GetSTDOUT(1000)
            print(stdout)
            state = process.GetState()
            if state == lldb.eStateStopped:
                # Get the first thread
                thread = process.GetThreadAtIndex(0)
                thread.StepOut()
                # Otherwise `actual_commands`isn't defined
                thread.StepOver()
                frame = thread.GetSelectedFrame()
                commands = frame.FindVariable('actual_commands')
                # We actually located our command queue
                # attempt to find the first member.
                child: lldb.SBValue = commands.GetChildMemberWithName('first')
                last = commands.GetChildMemberWithName('last')
                expected_generator = parse_expected_node()
                print_on_centre('Expected Command', 'Actual Command')
                while child.GetChildMemberWithName('next').GetValue() != last.GetChildMemberWithName('next').GetValue():
                    actual_node = parse_node(child.GetChildMemberWithName('current'))
                    expected_node = next(expected_generator)
                    print_on_centre(expected_node, actual_node)
                    child = child.GetChildMemberWithName('next')
                else:
                    # Afterwards
                    actual_node = parse_node(child.GetChildMemberWithName('current'))
                    try:
                        expected_node = next(expected_generator)
                    except StopIteration:
                        expected_node = ''
                    print_on_centre(expected_node, actual_node)
                    for expected_command in expected_generator:
                        print(expected_command)
        else:
            print('Failed to execute processs')
    else:
        print('Debugger creation failed')

