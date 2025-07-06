# Generate every single load instruction
from json import load, dump
from utils import get_tokens_except, sort_instructions, gen_register, gen_register_16
from itertools import repeat

registers = ["B", "C", "D", "E", "H", "L", "HL", "A"]

tokens = get_tokens_except(
    [
        *range(0x40, 0x80),
        0x02,
        0x06,
        0x0A,
        0x0E,
        0x12,
        0x16,
        0x1A,
        0x1E,
        0x22,
        0x26,
        0x2A,
        0x2E,
        0x32,
        0x36,
        0x3A,
        0x3E,
        0xE0,
        0xE2,
        0xEA,
        0xF0,
        0xF2,
        0xFA,
        *range(0xC1, 0xF2, 16),
        *range(0xC5, 0xF6, 16),
        0xF8,
        0xF9,
        *range(0x01, 0x32, 16),
        0x08
    ]
)

print(
    f"Starting inserting load operations with existing instruction set of {len(tokens)}"
)

opcode = 0x40

# First generate the large block between 0x40 and 0x7F, inclusive.
for dest_register in registers:
    dst_param = gen_register(dest_register)
    for source_register in registers:
        src_param = gen_register(source_register)
        instruction = {
            "instruction": f"0x{opcode:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
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
        dst_param = gen_register(dest_register)
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
                "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
                "duration_in_clock_cycles": (
                    3 if dest_register == "HL" or source_register == "HL" else 2
                ),
                "original_machine_code": instruction,
                "byte_length": 2,
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

# Now finally let us build the final block, this block goes is those instructions in
# 0xX2 and 0x0A, and what loads indirectly form 16-bit registers or loads TO them from A


A_REGISTER = gen_register("A")

registers = ["BC", "DE", "HL", "HL"]
ops = [
    "BEEMU_POST_LOAD_NOP",
    "BEEMU_POST_LOAD_NOP",
    "BEEMU_POST_LOAD_INCREMENT_INDIRECT_DESTINATION",
    "BEEMU_POST_LOAD_DECREMENT_INDIRECT_DESTINATION",
]

# Stands for least significant nibble btw.
for lsn in range(4):
    reg_16 = registers[lsn]
    register = {
        "pointer": True,
        "type": "BEEMU_PARAM_TYPE_REGISTER_16",
        "value": {"register_16": f"BEEMU_REGISTER_{reg_16}"},
    }

    # in 0xX2 it starts as R16 <- A
    dest = register
    src = A_REGISTER
    post_op = ops[lsn]
    for msn in [0x02, 0xA]:
        instruction = (lsn << 4) | msn
        token = {
            "instruction": f"0x{instruction:06X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
                "duration_in_clock_cycles": 2,
                "original_machine_code": instruction,
                "byte_length": 1,
                "params": {
                    "load_params": {
                        "source": src,
                        "dest": dest,
                        # Fun story...
                        "postLoadOperation": post_op,
                    }
                },
            },
        }
        # But we swap in 0xXA!
        dest, src = src, dest
        # the operation as well.
        post_op = post_op.replace("DESTINATION", "SOURCE")
        tokens.append(token)

# Finally the LDH and LD [imm16]s, written by hand for ease of use.

addr_param = {
    "pointer": True,
    "type": "BEEMU_PARAM_TYPE_UINT_8",
    "value": {"value": 0xBC},
}

addr16_param = {
    "pointer": True,
    "type": "BEEMU_PARAM_TYPE_UINT16",
    "value": {"value": 0xDEBC},
}


C_REGISTER_INDIRECT = {
    "pointer": True,
    "type": "BEEMU_PARAM_TYPE_REGISTER_8",
    "value": {"register_8": f"BEEMU_REGISTER_C"},
}


# LDH A, [a8]
tokens.append(
    {
        "instruction": f"0x00E0BC",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 3,
            "original_machine_code": 0x00E0BC,
            "byte_length": 2,
            "params": {
                "load_params": {
                    "source": A_REGISTER,
                    "dest": addr_param,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                }
            },
        },
    }
)


# LDH [a8], A
tokens.append(
    {
        "instruction": f"0x00F0BC",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 3,
            "original_machine_code": 0x00F0BC,
            "byte_length": 2,
            "params": {
                "load_params": {
                    "source": addr_param,
                    "dest": A_REGISTER,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                }
            },
        },
    }
)


# LDH [C], A
tokens.append(
    {
        "instruction": f"0x0000E2",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 2,
            "original_machine_code": 0x0000E2,
            "byte_length": 1,
            "params": {
                "load_params": {
                    "source": A_REGISTER,
                    "dest": C_REGISTER_INDIRECT,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                }
            },
        },
    }
)


# LDH A, [C]
tokens.append(
    {
        "instruction": f"0x0000F2",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 2,
            "original_machine_code": 0x0000F2,
            "byte_length": 1,
            "params": {
                "load_params": {
                    "source": C_REGISTER_INDIRECT,
                    "dest": A_REGISTER,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                }
            },
        },
    }
)

# LD [a16], A

tokens.append(
    {
        "instruction": f"0xEADEBC",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 4,
            "original_machine_code": 0xEADEBC,
            "byte_length": 3,
            "params": {
                "load_params": {
                    "source": A_REGISTER,
                    "dest": addr16_param,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                }
            },
        },
    }
)

# LD A, [a16]

tokens.append(
    {
        "instruction": f"0xFADEBC",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 4,
            "original_machine_code": 0xFADEBC,
            "byte_length": 3,
            "params": {
                "load_params": {
                    "source": addr16_param,
                    "dest": A_REGISTER,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP",
                }
            },
        },
    }
)

# Let's also add PUSH/POP series.

registers = [*map(lambda r: gen_register_16(r, False), ["BC", "DE", "HL", "AF"])]
stack_pointer = gen_register_16("SP", True)
stack_pointer_raw = gen_register_16("SP", False)
hl_register = gen_register_16("HL", False)


pop_opcodes = [*range(0xC1, 0xF2, 16)]
push_opcodes = [*range(0xC5, 0xF6, 16)]

pushes = [
    {
        "instruction": f"0x{opcode:06X}",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 4,
            "original_machine_code": opcode,
            "byte_length": 1,
            "params": {
                "load_params": {
                    "source": src,
                    "dest": dest,
                    "postLoadOperation": "BEEMU_POST_LOAD_DECREMENT_INDIRECT_DESTINATION",
                }
            },
        },
    }
    for opcode, src, dest in zip(push_opcodes, registers, repeat(stack_pointer, 4))
]

pops = [
    {
        "instruction": f"0x{opcode:06X}",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 3,
            "original_machine_code": opcode,
            "byte_length": 1,
            "params": {
                "load_params": {
                    "source": src,
                    "dest": dest,
                    "postLoadOperation": "BEEMU_POST_LOAD_INCREMENT_INDIRECT_SOURCE",
                }
            },
        },
    }
    for opcode, dest, src in zip(pop_opcodes, registers, repeat(stack_pointer, 4))
]

adjusted_sp_load = {
    "instruction": f"0x00F8F0",
    "token": {
        "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
        "duration_in_clock_cycles": 3,
        "original_machine_code": 0x00F8F0,
        "byte_length": 2,
        "params": {
            "load_params": {
                "source": stack_pointer_raw,
                "dest": hl_register,
                "postLoadOperation": "BEEMU_POST_LOAD_SIGNED_PAYLOAD_SUM",
                "auxPostLoadParameter": {
                    "pointer": False,
                    "type": "BEEMU_PARAM_TYPE_INT_8",
                    "value": {"signed_value": -16},
                }
            }
        },
    },
}

load_to_sp = {
    "instruction": f"0x0000F9",
    "token": {
        "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
        "duration_in_clock_cycles": 2,
        "original_machine_code": 0x0000F9,
        "byte_length": 1,
        "params": {
            "load_params": {
                "source": hl_register,
                "dest": stack_pointer_raw,
                "postLoadOperation": "BEEMU_POST_LOAD_NOP"
            }
        },
    },
}

# LD 16, imm16 block
registers = [*map(lambda r: gen_register_16(r, False), ["BC", "DE", "HL", "SP"])]
opcodes = range(0x01, 0x32, 16)

load_immediates = [];

for opcode, register in zip(opcodes, registers):
    load_immediates.append({
        "instruction": f"0x{opcode:02X}ABCD",
        "token": {
            "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
            "duration_in_clock_cycles": 3,
            "original_machine_code": (opcode << 16) + 0xABCD,
            "byte_length": 3,
            "params": {
                "load_params": {
                    "source": {
                        "pointer": False,
                        "type": "BEEMU_PARAM_TYPE_UINT16",
                        "value": {"value": 0xABCD}

                    },
                    "dest": register,
                    "postLoadOperation": "BEEMU_POST_LOAD_NOP"
                }
            },
        }
    })


save_sp = {
    "instruction": f"0x08ABCD",
    "token": {
        "type": "BEEMU_INSTRUCTION_TYPE_LOAD",
        "duration_in_clock_cycles": 5,
        "original_machine_code":0x08ABCD,
        "byte_length": 3,
        "params": {
            "load_params": {
                "source": stack_pointer_raw,
                "dest": {
                    "pointer": True,
                    "type": "BEEMU_PARAM_TYPE_UINT16",
                    "value": {"value": 0xABCD}
                },
                "postLoadOperation": "BEEMU_POST_LOAD_NOP"
            }
        },
    }
}

tokens.extend([*pushes, *pops])

tokens.extend([load_to_sp, adjusted_sp_load])

tokens.extend(load_immediates)

tokens.append(save_sp)

sort_instructions(tokens)

with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all loads, reaching a total of {len(tokens)}.")
