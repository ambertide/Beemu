"""
Print out the emitted bytecodes for a specific instruction
by the `beemu_parser_parse` function using `lldb`
"""

import lldb
import os
from sys import argv

opcode = argv[1]

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

def parse_node(instr: lldb.SBValue) -> None:
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
        write_value = write_value_info.GetChildMemberWithName('value').GetChildMemberWithName(write_value_field).GetValue()
        print(f'WRITE {write_target} {write_value}')
    elif type_ == 'BEEMU_COMMAND_HALT':
        print('HALT')
    else:
        print(instr)
        print('UNSUPPORTED')



if __name__ == '__main__':
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
                while child.GetChildMemberWithName('next').GetValue() != last.GetChildMemberWithName('next').GetValue():
                    parse_node(child.GetChildMemberWithName('current'))
                    child = child.GetChildMemberWithName('next')
                else:
                    parse_node(child.GetChildMemberWithName('current'))
        else:
            print('Failed to execute processs')
    else:
        print('Debugger creation failed')

