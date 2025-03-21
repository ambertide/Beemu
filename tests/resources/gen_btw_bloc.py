# Generates the BIT, RES and SET operation test codes since
# they are VERY repetative, ovverrides the tokens.json in the process.
from json import load, dump

with open("tokens.json") as file:
    test_data = load(file)["tokens"]

# First, freshly create the data.

BLOCK_START = 0xCB40
BLOCK_END = 0xCBFF

ops = []


def calc_target(opcode: int) -> tuple[str, dict]:
    if opcode & 0x000F in [0x6, 0xE]:
        # (HL) bloc btw.
        return "REG_16_PTR", {
            "pointer": True,
            "type": "BEEMU_PARAM_TYPE_REGISTER_16",
            "value": {"register_16": "BEEMU_REGISTER_HL"},
        }
    else:
        return "REG_8", {
            "pointer": False,
            "type": "BEEMU_PARAM_TYPE_REGISTER_8",
            "value": {
                "register_8": [
                    "BEEMU_REGISTER_B",
                    "BEEMU_REGISTER_C",
                    "BEEMU_REGISTER_D",
                    "BEEMU_REGISTER_E",
                    "BEEMU_REGISTER_H",
                    "BEEMU_REGISTER_L",
                    "BEEMU_REGISTER_A",
                    "BEEMU_REGISTER_A",
                    "BEEMU_REGISTER_B",
                    "BEEMU_REGISTER_C",
                    "BEEMU_REGISTER_D",
                    "BEEMU_REGISTER_E",
                    "BEEMU_REGISTER_H",
                    "BEEMU_REGISTER_L",
                    "BEEMU_REGISTER_A",
                    "BEEMU_REGISTER_A",
                ][opcode & 0x000F]
            },
        }


def calc_clock_cycle(op: str, reg_type: str) -> int:
    if reg_type == "REG_8":
        return 2
    else:
        if op == "BEEMU_BIT_OP_BIT":
            return 3
        else:
            return 4


for opcode in range(BLOCK_START, BLOCK_END + 1):
    op = "BEEMU_BIT_OP_BIT"
    if 0xCBC0 > opcode >= 0xCB80:
        op = "BEEMU_BIT_OP_RES"
    elif opcode >= 0xCBC0:
        op = "BEEMU_BIT_OP_SET"

    baseline = 0xCB40

    if opcode == 0xCB80 or opcode == 0xCBC0:
        baseline = opcode

    reg_type, target = calc_target(opcode)
    clock_cycle = calc_clock_cycle(op, reg_type)
    ops.append(
        {
            "instruction": f"0x{opcode:4X}",
            "token": {
                "type": "BEEMU_INSTRUCTION_TYPE_BITWISE",
                "duration_in_clock_cycles": clock_cycle,
                "is_16": True,
                "payload": {
                    "bitwise_params": {
                        "operation": op,
                        "bit_number": ((opcode - baseline) // 8),
                        "target": target,
                    }
                },
            },
        }
    )

# Now that we calculated the operations, let's insert them to the
# File.

old_data_filtered = [
    inst for inst in test_data if int(inst["instruction"], 16) < 0xCB40
]

if len(old_data_filtered) < len(test_data):
    print("Found old entries for opcodes above 0xCB40 and removed it.")

new_data = [*old_data_filtered, *ops]

test_data = {"tokens": new_data}


with open("tokens.json", "w") as file:
    dump(test_data, file, indent="\t")

print(f"Inserted {len(ops)} amount of data.")
