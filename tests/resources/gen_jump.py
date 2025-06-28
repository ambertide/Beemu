from json import dump

from tests.resources.utils import get_tokens_except, sort_instructions

jr_conditional = [*range(0x20, 0x39, 8)]
rst_instructions = [*range(0xC7, 0x100, 8)]
conditional_jp_instructions = [*range(0xC2, 0xDB, 8)]

tokens = get_tokens_except([
    0x18,
    *jr_conditional,
    *rst_instructions,
    *conditional_jp_instructions,
    0xC3
])

conditions = [
    "BEEMU_JUMP_IF_NOT_ZERO",
    "BEEMU_JUMP_IF_ZERO",
    "BEEMU_JUMP_IF_NOT_CARRY",
    "BEEMU_JUMP_IF_CARRY"
]


# JR INSTRUCTIONS START


# first unconditional
tokens.append({
    "instruction": f"0x0018F0",
    "token": {
        "type": "BEEMU_INSTRUCTION_TYPE_JUMP",
        "duration_in_clock_cycles": 3,
        "original_machine_code": (0x18 << 8) + 0xF0,
        "byte_length": 2,
        "params": {
            "jump_params": {
                "is_conditional": False,
                "is_relative": True,
                "enable_interrupts": False,
                "type": "BEEMU_JUMP_TYPE_JUMP",
                "condition": "BEEMU_JUMP_IF_NO_CONDITION",
                "param": {
                    "pointer": False,
                    "type": "BEEMU_PARAM_TYPE_INT_8",
                    "value": { "signed_value": -16 }
                }
            }
        },
    }
})
# then conditional
for opcode, condition in zip(jr_conditional, conditions):
    tokens.append({
        "instruction": f"0x00{opcode:02X}F0",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_JUMP",
            "duration_in_clock_cycles": 3,
            "original_machine_code": (opcode << 8) + 0xF0,
            "byte_length": 2,
            "params": {
                "jump_params": {
                    "is_conditional": True,
                    "is_relative": True,
                    "enable_interrupts": False,
                    "type": "BEEMU_JUMP_TYPE_JUMP",
                    "condition": condition,
                    "param": {
                        "pointer": False,
                        "type": "BEEMU_PARAM_TYPE_INT_8",
                        "value": { "signed_value": -16 }
                    }
                }
            },
        }
    })

# RST INSTRUCTIONS START
mem_addresses = range(0x00, 0x39, 8)

for opcode, mem_address in zip(rst_instructions, mem_addresses):
    tokens.append({
        "instruction": f"0x{opcode:06X}",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_JUMP",
            "duration_in_clock_cycles": 4,
            "original_machine_code": opcode,
            "byte_length": 1,
            "params": {
                "jump_params": {
                    "is_conditional": False,
                    "is_relative": False,
                    "enable_interrupts": False,
                    "type": "BEEMU_JUMP_TYPE_RST",
                    "condition": "BEEMU_JUMP_IF_NO_CONDITION",
                    "param": {
                       "pointer": False,
                        "type": "BEEMU_PARAM_TYPE_UINT16",
                        "value": { "value": mem_address }
                    }
                }
            },
        }
    })

for opcode, condition in zip(conditional_jp_instructions, conditions):
    tokens.append({
        "instruction": f"0x{opcode:02X}ABCD",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_JUMP",
            "duration_in_clock_cycles": 4,
            "original_machine_code": (opcode << 16) + 0xABCD,
            "byte_length": 3,
            "params": {
                "jump_params": {
                    "is_conditional": True,
                    "is_relative": False,
                    "enable_interrupts": False,
                    "type": "BEEMU_JUMP_TYPE_JUMP",
                    "condition": condition,
                    "param": {
                        "pointer": False,
                        "type": "BEEMU_PARAM_TYPE_UINT16",
                        "value": { "value": 0xABCD }
                    }
                }
            },
        }
    })

# Unconditional variant of jump

tokens.append({
    "instruction": f"0xC3ABCD",
    "token": {
        "type": "BEEMU_INSTRUCTION_TYPE_JUMP",
        "duration_in_clock_cycles": 4,
        "original_machine_code": (0xC3 << 16) + 0xABCD,
        "byte_length": 3,
        "params": {
            "jump_params": {
                "is_conditional": False,
                "is_relative": False,
                "enable_interrupts": False,
                "type": "BEEMU_JUMP_TYPE_JUMP",
                "condition": "BEEMU_JUMP_IF_NO_CONDITION",
                "param": {
                    "pointer": False,
                    "type": "BEEMU_PARAM_TYPE_UINT16",
                    "value": { "value": 0xABCD }
                }
            }
        },
    }
})


if __name__ == '__main__':
    sort_instructions(tokens)
    with open("tokens.json", "w") as file:
        dump({"tokens": tokens}, file, indent="\t")

    print(f"Inserted all jumps, reaching a total of {len(tokens)}.")