from json import dump

from tests.resources.utils import get_tokens_except, sort_instructions

rst_instructions = [*range(0xC7, 0x100, 8)]

tokens = get_tokens_except([*rst_instructions])

# RST INSTRUCTIONS START
mem_addresses = range(0x00, 0x39, 8)

for opcode, mem_address in zip(rst_instructions, mem_addresses):
    tokens.append({
        "instruction": f"{opcode:06X}",
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
                       "pointer": True,
                        "type": "BEEMU_PARAM_TYPE_UINT16",
                        "value": { "value": mem_address }
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