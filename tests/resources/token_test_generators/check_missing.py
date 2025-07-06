from json import load
from pprint import pprint

from tests.resources.utils import get_tokens_except, get_opcode

if __name__ == '__main__':
    opcode_space = set(range(0x00, 0x100))
    opcode_space = opcode_space.difference([0xD3, 0xE3, 0xE4, 0xF4, 0xCB, 0xDB, 0xEB, 0xEC, 0xFC, 0xDD, 0xED, 0xFD])

    tokens = get_tokens_except([])
    opcodes = [get_opcode(int(token["instruction"], base=16)) for token in tokens]

    remaining_opcodes = opcode_space.difference(opcodes)

    pprint([f"0x{opcode:02X}" for opcode in remaining_opcodes])

    pprint(f"{len(remaining_opcodes)} remained to tokenize.")
