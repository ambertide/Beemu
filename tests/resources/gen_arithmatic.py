from json import dump
from utils import get_tokens_except, gen_register, sort_instructions
from itertools import cycle, repeat

arithmatics = [*range(0x80, 0xC0)]
inc_dec = [*range(0x04, 0x3D, 8), *range(0x05, 0x3F, 8)]
arithmatic_direct = [*range(0xC6, 0xFF, 8)]

tokens = get_tokens_except([*arithmatics, *inc_dec, *arithmatic_direct])


a_register = gen_register("A")
registers = [*map(lambda r: gen_register(r), ["B", "C", "D", "E", "H", "L", "HL", "A"])]
operations = [
    "BEEMU_OP_ADD",
    "BEEMU_OP_ADC",
    "BEEMU_OP_SUB",
    "BEEMU_OP_SBC",
    "BEEMU_OP_AND",
    "BEEMU_OP_OR",
    "BEEMU_OP_CP",
    "BEEMU_OP_XOR",
]

# Encode mainline instructions first.

for opcode, subop, source_register in zip(
    arithmatics,
    (op for optype in operations for op in repeat(optype, 8)),
    cycle(registers),
):
    tokens.append(
        {
            "instruction": f"0x{opcode:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8",
                "duration_in_clock_cycles": (
                    1 if source_register["type"] == "BEEMU_PARAM_TYPE_REGISTER_8" else 2
                ),
                "original_machine_code": opcode,
                "params": {
                    "arithmatic_params": {
                        "operation": subop,
                        "dest_or_first": a_register,
                        "source_or_second": source_register,
                    }
                },
            },
        }
    )

# Secondarilly, encode the INC and DEC instructions
# I have seemingly gave an early decision to encode these as
# ADD and SUBs with UINT8 1, while this makes some sense
# I have this ominious feeling that this is a horrible mistake...

# Oh well...

for operation, opcode_range in (
    ("BEEMU_OP_ADD", range(0x04, 0x3D, 8)),
    ("BEEMU_OP_SUB", range(0x05, 0x3F, 8)),
):
    for opcode, dest_register in zip(opcode_range, registers):
        tokens.append(
            {
                "instruction": f"0x{opcode:06X}",
                "token": {
                    "duration_in_clock_cycles": (
                        1
                        if dest_register["type"] == "BEEMU_PARAM_TYPE_REGISTER_8"
                        else 3
                    ),
                    "original_machine_code": opcode,
                    "params": {
                        "arithmatic_params": {
                            "operation": operation,
                            "dest_or_first": dest_register,
                            "source_or_second": {
                                "pointer": False,
                                "type": "BEEMU_PARAM_TYPE_UINT_8",
                                "value": {"value": 1},
                            },
                        }
                    },
                },
            }
        )

# Add the direct arithmatics of type OPERATION A, n8
for operation, opcode in zip(operations, range(0xC6, 0xFF, 8)):
    full_instruction = (opcode << 8) | 0xDF
    tokens.append(
        {
            "instruction": f"{full_instruction:06X}",
            "token": {
                "duration_in_clock_cycles": (
                    1 if source_register["type"] == "BEEMU_PARAM_TYPE_REGISTER_8" else 2
                ),
                "original_machine_code": opcode,
                "params": {
                    "arithmatic_params": {
                        "operation": operation,
                        "dest_or_first": a_register,
                        "source_or_second": {
                            "pointer": False,
                            "type": "BEEMU_PARAM_TYPE_UINT_8",
                            "value": {"value": 0xDF},
                        },
                    }
                },
            },
        }
    )

sort_instructions(tokens)
with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all 8 bit arithmatics, reaching a total of {len(tokens)}.")
