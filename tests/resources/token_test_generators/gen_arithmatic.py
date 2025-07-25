from json import dump

from utils import get_tokens_except, gen_register, sort_instructions, gen_register_16
from itertools import cycle, repeat

arithmatics = [*range(0x80, 0xC0)]
inc_dec = [*range(0x04, 0x3D, 8), *range(0x05, 0x3F, 8)]
arithmatic_direct = [*range(0xC6, 0xFF, 8)]
weird = [*range(0x27, 0x40, 8)]
inc_dec16 = [*range(0x03, 0x3C, 8)]
add16 = [*range(0x09, 0x3A, 16)]
sp_adjust = [0xE8]


tokens = get_tokens_except([*arithmatics, *inc_dec, *arithmatic_direct, *weird, *inc_dec16, *add16, *sp_adjust])


a_register = gen_register("A")
registers = [*map(lambda r: gen_register(r), ["B", "C", "D", "E", "H", "L", "HL", "A"])]
operations = [
    "BEEMU_OP_ADD",
    "BEEMU_OP_ADC",
    "BEEMU_OP_SUB",
    "BEEMU_OP_SBC",
    "BEEMU_OP_AND",
    "BEEMU_OP_XOR",
    "BEEMU_OP_OR",
    "BEEMU_OP_CP",
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
                "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
                "duration_in_clock_cycles": (
                    1 if source_register["type"] == "BEEMU_PARAM_TYPE_REGISTER_8" else 2
                ),
                "original_machine_code": opcode,
                "byte_length": 1,
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
                    "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
                    "duration_in_clock_cycles": (
                        1
                        if dest_register["type"] == "BEEMU_PARAM_TYPE_REGISTER_8"
                        else 3
                    ),
                    "original_machine_code": opcode,
                    "byte_length": 1,
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
                "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
                "duration_in_clock_cycles": 2,
                "original_machine_code": full_instruction,
                "byte_length": 2,
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


# "Weird" arithmatics block.
weird_operations = ["BEEMU_OP_DAA", "BEEMU_OP_CPL", "BEEMU_OP_SCF", "BEEMU_OP_CCF"]
for operation, opcode in zip(weird_operations, range(0x27, 0x40, 8)):
    tokens.append(
        {
            "instruction": f"0x{opcode:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
                "duration_in_clock_cycles": 1,
                "original_machine_code": opcode,
                "byte_length": 1,
                "params": {
                    "arithmatic_params": {
                        "operation": operation,
                        "dest_or_first": a_register,
                        "source_or_second": a_register,
                    }
                },
            },
        }
    )

bc_register = gen_register_16('BC')
de_register = gen_register_16('DE')
hl_register = gen_register_16('HL')
sp_register = gen_register_16('SP')

registers = [
    bc_register,
    bc_register,
    de_register,
    de_register,
    hl_register,
    hl_register,
    sp_register,
    sp_register
]

for opcode, operation, register in  zip(range(0x03, 0x3C, 8), cycle(['BEEMU_OP_ADD', 'BEEMU_OP_SUB']), registers):
    tokens.append(
        {
            "instruction": f"0x{opcode:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
                "duration_in_clock_cycles": 2,
                "original_machine_code": opcode,
                "byte_length": 1,
                "params": {
                    "arithmatic_params": {
                        "operation": operation,
                        "dest_or_first": register,
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


# ADD HL, Reg16 block
registers = [
    bc_register,
    de_register,
    hl_register,
    sp_register,
]

opcodes = range(0x09, 0x3A, 16)

for opcode, register in zip(opcodes, registers):
    tokens.append(
        {
            "instruction": f"0x{opcode:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
                "duration_in_clock_cycles": 2,
                "original_machine_code": opcode,
                "byte_length": 1,
                "params": {
                    "arithmatic_params": {
                        "operation": "BEEMU_OP_ADD",
                        "dest_or_first": hl_register,
                        "source_or_second": register
                    }
                },
            },
        }
    )

# SP Adjust

tokens.append({
    "instruction": f"0x00E8FE",
    "token": {
        "type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC",
        "duration_in_clock_cycles": 4,
        "original_machine_code": 0xE8FE,
        "byte_length": 2,
        "params": {
            "arithmatic_params": {
                "operation": "BEEMU_OP_ADD",
                "dest_or_first": sp_register,
                "source_or_second": {
                    "pointer": False,
                    "type": "BEEMU_PARAM_TYPE_INT_8",
                    "value": {"signed_value": -2}
                }
            }
        },
    }
})

sort_instructions(tokens)
with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all bit arithmatics, reaching a total of {len(tokens)}.")
