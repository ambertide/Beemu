# These are the special ROT SHIFTS that are in the mainline.
from json import dump

from tests.resources.utils import get_tokens_except, sort_instructions

insts = [*range(0x07, 0x20, 8)]

tokens = get_tokens_except(insts)

for token in tokens:
    # This flag was added with this instruction so.
    if token['token']['type'] == 'BEEMU_INSTRUCTION_TYPE_ROT_SHIFT':
        token['token']['params']['rot_shift_params']['set_flags_to_zero'] = False

for opcode in insts:
    tokens.append({
        "instruction": f"0x{opcode:06X}",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_ROT_SHIFT",
            "duration_in_clock_cycles": 1,
            "original_machine_code": opcode,
            "byte_length": 1,
            "params": {
                "rot_shift_params": {
                    "through_carry": opcode < 0x10,
                    "operation": "BEEMU_ROTATE_OP",
                    "direction": "BEEMU_LEFT_DIRECTION" if opcode in [0x07, 0x17]  else "BEEMU_RIGHT_DIRECTION",
                    "target": {
                        "pointer": False,
                        "type": "BEEMU_PARAM_TYPE_REGISTER_8",
                        "value": {
                            "register_8": "BEEMU_REGISTER_A"
                        }
                    },
                    "set_flags_to_zero": True
                }
            }
        }
    })

if __name__ == '__main__':
    sort_instructions(tokens)
    with open("tokens.json", "w") as file:
        dump({"tokens": tokens}, file, indent="\t")

    print(f"Inserted all RRA/RLA, reaching a total of {len(tokens)}.")