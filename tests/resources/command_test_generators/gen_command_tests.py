from gen_arithmatic_tests import emit_arithmetic_tests
from gen_bitwise_tests import emit_bitwise_tests
from gen_load_tests import emit_load_tests

from json import dump

from tests.resources.command_test_generators.utils import get_tokens_of_type
from tests.resources.token_test_generators.utils import sort_instructions

test_emitters = {
    'ARITHMETIC': emit_arithmetic_tests,
    'BITWISE': emit_bitwise_tests,
    'LOAD': emit_load_tests
}

if __name__ == '__main__':
    tests = []
    for token_type, emitter in test_emitters.items():
        tokens = get_tokens_of_type(f"BEEMU_INSTRUCTION_TYPE_{token_type}")
        otl = len(tests)
        tests.extend(emitter(tokens))
        print(f"Emitted {len(tests) - otl} {token_type.lower()} instructions")


    sort_instructions(tests, lambda test: test["token"]["original_machine_code"])

    with open('../command_tests.json', 'w') as f:
        dump({ 'commands': tests }, f, indent='\t')