# Generate system instructions except 0x76
from json import load, dump
from utils import get_tokens_except, sort_instructions, gen_register, gen_register_16
from itertools import repeat


tokens = get_tokens_except(
    [
        0x00,
        0x10,
        0xF3,
        0xFB
    ]
)

print(
    f"Starting inserting load operations with existing instruction set of {len(tokens)}"
)

system_instructions = [
    {
        "instruction": f"0x000000",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_CPU_CONTROL",
            "duration_in_clock_cycles": 1,
            "original_machine_code":0x000000,
            "byte_length": 1,
            "params": {
                "system_op": "BEEMU_CPU_OP_NOP"
            },
        }
    },
    {
        "instruction": f"0x001000",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_CPU_CONTROL",
            "duration_in_clock_cycles": 1,
            "original_machine_code":0x001000,
            "byte_length": 2,
            "params": {
                "system_op": "BEEMU_CPU_OP_STOP"
            },
        }
    },
    # 0x76 is generated on the load part because of a special case with
    # the for loop.
    {
        "instruction": f"0x0000F3",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_CPU_CONTROL",
            "duration_in_clock_cycles": 1,
            "original_machine_code":0x0000F3,
            "byte_length": 1,
            "params": {
                "system_op": "BEEMU_CPU_OP_DISABLE_INTERRUPTS"
            },
        }
    },
    {
        "instruction": f"0x0000FB",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_CPU_CONTROL",
            "duration_in_clock_cycles": 1,
            "original_machine_code":0x0000FB,
            "byte_length": 1,
            "params": {
                "system_op": "BEEMU_CPU_OP_ENABLE_INTERRUPTS"
            },
        }
    },
]

tokens.extend(system_instructions)

sort_instructions(tokens)

with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all system instructions except 0x76, reaching a total of {len(tokens)}.")
