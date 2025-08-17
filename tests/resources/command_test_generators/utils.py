import dataclasses
from typing import Iterable
from json import load

from tests.resources.token_test_generators.utils import get_opcode


def get_tokens_in_range(_range: Iterable[int]) -> list[dict]:
    concreate_range = [*_range]
    with open('../tokens.json') as file:
        tokens = load(file)['tokens']
    return [token for token in tokens if get_opcode(token['token']['original_machine_code']) in concreate_range]


def get_tokens_of_type(type_: str) -> list[dict]:
    with open('../tokens.json') as file:
        tokens = load(file)['tokens']
    return [token for token in tokens if token['token']['type'] == type_]

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
    def ir(cls, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_INTERNAL",
                    "target": {
                        "internal_target": "BEEMU_INTERNAL_WRITE_TARGET_INSTRUCTION_REGISTER"
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
    def pc(cls, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_INTERNAL",
                    "target": {
                        "internal_target": "BEEMU_INTERNAL_WRITE_TARGET_PROGRAM_COUNTER"
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
        if len(register) == 2:
            return {
                "type": "BEEMU_COMMAND_WRITE",
                "write": {
                    "target": {
                        "type": "BEEMU_WRITE_TARGET_REGISTER_16",
                        "target": {
                            "register_16": f"BEEMU_REGISTER_{register}"
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

    @classmethod
    def memory(cls, addr: int, value: int) -> dict:
        return {
            "type": "BEEMU_COMMAND_WRITE",
            "write": {
                "target": {
                    "type": "BEEMU_WRITE_TARGET_MEMORY_ADDRESS",
                    "target": {
                        "mem_addr": addr
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


@dataclasses.dataclass
class Param:
    type: str
    pointer: bool
    value: str | int

    @classmethod
    def from_dict(cls, data: dict) -> "Param":
        return Param(
            data['type'],
            data['pointer'],
            [*data['value'].values()][0]
        )

    @property
    def register(self) -> str:
        return self.value.replace('BEEMU_REGISTER_', '')

def emit_m1_cycle(token: dict, override_opcode: int = 0) -> list[dict]:
    return [
        WriteTo.pc(0x01),
        WriteTo.ir(get_opcode(token['original_machine_code']) if not override_opcode else override_opcode),
        Halt.cycle(),
    ]