# Generate every single load instruction
from json import load, dump
from byte_utils import get_opcode, sort_instructions

registers = ["B", "C", "D", "E", "H", "L", "HL", "A"]

with open("tokens.json") as file:
    data = load(file)
    tokens: list[dict] = data["tokens"]


to_be_removed = []
for token in tokens:
    opcode = get_opcode(token["token"]["original_machine_code"])

    if (0x7F >= opcode >= 0x40) or opcode in [
        0x06,
        0x0E,
        0x16,
        0x1E,
        0x26,
        0x2E,
        0x36,
        0x3E,
    ]:
        to_be_removed.append(token["token"]["original_machine_code"])

tokens[:] = [
    t for t in tokens if t["token"]["original_machine_code"] not in to_be_removed
]

print(f"Removed {len(to_be_removed)} tokens already existing.")
print(
    f"Starting inserting load operations with existing instruction set of {len(tokens)}"
)

opcode = 0x40

# First generate the large block between 0x40 and 0x7F, inclusive.
for dest_register in registers:
    dst_param = {
        "pointer": dest_register == "HL",
        "type": (
            "BEEMU_PARAM_TYPE_REGISTER_16"
            if dest_register == "HL"
            else "BEEMU_PARAM_TYPE_REGISTER_8"
        ),
        "value": (
            {"register_16": f"BEEMU_REGISTER_{dest_register}"}
            if dest_register == "HL"
            else {"register_8": f"BEEMU_REGISTER_{dest_register}"}
        ),
    }
    for source_register in registers:
        src_param = {
            "pointer": source_register == "HL",
            "type": (
                "BEEMU_PARAM_TYPE_REGISTER_16"
                if source_register == "HL"
                else "BEEMU_PARAM_TYPE_REGISTER_8"
            ),
            "value": (
                {"register_16": f"BEEMU_REGISTER_{source_register}"}
                if source_register == "HL"
                else {"register_8": f"BEEMU_REGISTER_{source_register}"}
            ),
        }
        instruction = {
            "instruction": f"0x{opcode:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_LOAD_8",
                "duration_in_clock_cycles": (
                    2 if dest_register == "HL" or source_register == "HL" else 1
                ),
                "original_machine_code": opcode,
                "byte_length": 1,
                "params": {
                    "load_params": {
                        "source": src_param,
                        "dest": dst_param,
                        "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                    }
                },
            },
        }

        if opcode == 0x76:
            instruction["token"] = {
                "type": "BEEMU_INSTRUCTION_TYPE_CPU_CONTROL",
                "duration_in_clock_cycles": 1,
                "original_machine_code": opcode,
                "byte_length": 1,
                "params": {"system_op": "BEEMU_CPU_OP_HALT"},
            }
        tokens.append(instruction)
        tokens.sort(key=lambda i: i["token"]["original_machine_code"])
        opcode += 1

# Now generate the weirdly distributed vertical blocks in 0xX6 and 0xXE where X is variable between
# 0x0 and 0x4

register_index: int = 0
for lsb in range(4):
    # 0xX section.
    for msb in [0x6, 0xE]:
        dest_register = registers[register_index]
        instruction = (lsb << 4) | msb
        dst_param = {
            "pointer": dest_register == "HL",
            "type": (
                "BEEMU_PARAM_TYPE_REGISTER_16"
                if dest_register == "HL"
                else "BEEMU_PARAM_TYPE_REGISTER_8"
            ),
            "value": (
                {"register_16": f"BEEMU_REGISTER_{dest_register}"}
                if dest_register == "HL"
                else {"register_8": f"BEEMU_REGISTER_{dest_register}"}
            ),
        }
        # These always take an imm8 as their source, oh, wait, this is interesting,
        # because then we need to add this to the instruction as well, shite...
        # Let's just add the instruction twice? this way the imm is also test variable.
        # Huzzah!
        # Hm however, this generates another bullshit problem where these will be pushed in between
        # existing texts, uh oh..
        # should probably add a canonical key.
        immediate_number = instruction
        instruction <<= 8
        instruction |= immediate_number

        src_param = {
            "pointer": False,
            # Arguably funny that this doesn't matter, like, *at all*
            # Load doesn't care if you are *U*int.
            "type": "BEEMU_PARAM_TYPE_UINT_8",
            "value": {"value": immediate_number},
        }

        instruction = {
            "instruction": f"0x{instruction:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_LOAD_8",
                "duration_in_clock_cycles": (
                    3 if dest_register == "HL" or source_register == "HL" else 2
                ),
                "original_machine_code": instruction,
                "byte_length": 3,
                "params": {
                    "load_params": {
                        "source": src_param,
                        "dest": dst_param,
                        "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                    }
                },
            },
        }

        tokens.append(instruction)
        register_index += 1

sort_instructions(tokens)

with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all loads, reaching a total of {len(tokens)}.")
