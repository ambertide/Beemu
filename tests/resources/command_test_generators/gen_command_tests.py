from gen_arithmatic_tests import emit_arithmetic_tests
from json import dump

from tests.resources.token_test_generators.utils import sort_instructions

test_emitters = [
    emit_arithmetic_tests
]

if __name__ == '__main__':
    tests = []
    for emitter in test_emitters:
        tests.extend(emitter())
    sort_instructions(tests, lambda test: test["token"]["original_machine_code"])
    with open('../command_tests.json', 'w') as f:
        dump({ 'commands': tests }, f, indent='\t')