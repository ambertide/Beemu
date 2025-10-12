from tests.resources.command_test_generators.utils import Param

def emit_jump_tests(tokens) -> list[dict]:
    tests = []
    for token in tokens:
        emitted_token = token['token']
        jp_params = emitted_token['params']['jump_params']
        jp_type = emitted_token['type']
        is_relative = jp_params['is_relative']
        is_conditional = jp_params['is_conditional']
        param = Param.from_dict(jp_params['param'])
        processor = 'default'
        match (jp_type, is_relative, is_conditional):
            case _:
                print(token['instruction'])
                continue
        tests.append({
            'token': emitted_token,
            'processor': processor,
            'command_queue': command_queue
        })
    return tests