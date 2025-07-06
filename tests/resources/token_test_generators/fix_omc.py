# This script sets the .original_machine_code fields in the tokens.json
# because it is brutal to do by hand fr fr.

from json import load, dump

with open("tokens.json") as file:
    test_data = load(file)

for token in test_data["tokens"]:
    instruction_in_hex = token["instruction"]
    instruction_in_decimal = int(instruction_in_hex, 16)
    token["token"]["original_machine_code"] = instruction_in_decimal

with open("tokens.json", "w") as file:
    dump(test_data, file, indent="\t")
