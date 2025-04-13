# Generate every single load instruction
from json import load, dump

registers = ["B", "C", "D", "E", "H", "L", "HL", "A"]

opcode = 0x40

with open("tokens.json") as file:
    data = load(file)
    tokens: list[dict] = data["tokens"]


to_be_removed = []
for token in tokens:
    if 0x7F >= token["token"]["original_machine_code"] >= 0x40:
        to_be_removed.append(token["token"]["original_machine_code"])

tokens[:] = [
    t for t in tokens if t["token"]["original_machine_code"] not in to_be_removed
]

print(f"Removed {len(to_be_removed)} tokens already existing.")
print(
    f"Starting inserting load operations with existing instruction set of {len(tokens)}"
)

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
                "params": {"load_params": {"source": src_param, "dest": dst_param}},
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

with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all loads, reaching a total of {len(tokens)}.")
