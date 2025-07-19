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

class WriteTo:
    @classmethod
    def data_bus(cls, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_INTERNAL",
                    "target": {
                        "internal_target": "BEEMU_INTERNAL_WRITE_TARGET_DATA_BUS"
                    }
                },
                "value": {
                    "is_16": False,
                    "value": {
                        "byte_value": value
                    }
                }
            }

        }

    @classmethod
    def address_bus(cls, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_INTERNAL",
                    "target": {
                        "internal_target": "BEEMU_INTERNAL_WRITE_TARGET_ADDRESS_BUS"
                    }
                },
                "value": {
                    "is_16": True,
                    "value": {
                        "double_value": value
                    }
                }
            }
        }

    @classmethod
    def register(cls, register: str, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_REGISTER_8",
                    "target": {
                        "register_8": f"BEEMU_REGISTER_{register}"
                    }
                },
                "value": {
                    "is_16": False,
                    "value": {
                        "byte_value": value
                    }
                }
            }
        }

    @classmethod
    def flag(cls, flag: str, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_FLAG",
                    "target": {
                        "flag": f"BEEMU_FLAG_{flag}"
                    }
                },
                "value": {
                    "is_16": False,
                    "value": {
                        "byte_value": value
                    }
                }
            }
        }

class Halt:
    @classmethod
    def cycle(cls) -> dict:
       return {
           "type": "BEEMU_COMMAND_HALT",
           "halt": {
               "is_cycle_terminator": True
           }
       }
