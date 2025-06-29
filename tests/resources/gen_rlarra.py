# These are the special ROT SHIFTS that are in the mainline.
from json import dump

from tests.resources.utils import get_tokens_except, sort_instructions

insts = [*range(0x07, 0x20, 8)]

tokens = get_tokens_except(insts)

for token in tokens:
    # This flag was added with this instruction so.
    if token['token']['type'] == 'BEEMU_INSTRUCTION_TYPE_ROT_SHIFT':
        token['token']['params']['rot_shift_params']['set_flags_to_zero'] = False


if __name__ == '__main__':
    sort_instructions(tokens)
    with open("tokens.json", "w") as file:
        dump({"tokens": tokens}, file, indent="\t")

    print(f"Inserted all RRA/RLA, reaching a total of {len(tokens)}.")