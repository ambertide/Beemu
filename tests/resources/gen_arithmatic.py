from json import dump
from utils import get_tokens_except, gen_register, sort_instructions
from itertools import cycle, repeat

arithmatics = [*range(0x80, 0xC0)]

tokens = get_tokens_except(arithmatics)


a_register = gen_register('A')
registers = [*map(lambda r: gen_register(r), ['B', 'C', 'D', 'E', 'H', 'L', 'HL', 'A'])]
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


for opcode, subop, source_register in zip(
	arithmatics,
	(op for optype in operations for op in repeat(optype, 8)),
	cycle(registers)
):
	tokens.append({
		"instruction": f"0x{opcode:06X}",
		"token": {
			"type": "BEEMU_INSTRUCTION_TYPE_ARITHMATIC_8",
			"duration_in_clock_cycles": 1 if source_register['type'] == "BEEMU_PARAM_TYPE_REGISTER_8" else 2,
			"original_machine_code": opcode,
			"params": {
				"arithmatic_params": {
					"operation": subop,
					"dest_or_first": a_register,
					"source_or_second": source_register
				}
			}
		}
	})	


sort_instructions(tokens)
with open("tokens.json", "w") as file:
    dump({"tokens": tokens}, file, indent="\t")

print(f"Inserted all 8 bit arithmatics, reaching a total of {len(tokens)}.")
