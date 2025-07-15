from typing import Iterable
from json import load
def get_tokens_in_range(_range: Iterable[int]) -> list[dict]:
    concreate_range = [*_range]
    with open('../tokens.json') as file:
        tokens = load(file)['tokens']
    return [token for token in tokens if int(token['instruction'], base=16) in concreate_range]


def reduce_flags(carry: int, parity: int, aux: int, zero: int):
    """
    Calculate flag compressed value from concrete values.
    :param carry: C flag
    :param parity: H flag
    :param aux:  N flag
    :param zero: Z flag
    :return: 
    """
    return int(f"0x0000{1 if carry else 0}{1 if parity else 0}{1 if aux else 0}{1 if zero else 0}", base=16)
