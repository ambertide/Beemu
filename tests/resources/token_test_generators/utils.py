# Who knew byte operations could be complicated?
# hint: not me...


from json import load


def get_opcode(instruction: int) -> int:
    """This is the byte(s) used to distinguish instr category.

    Gameboy instructions are variable in their byte count,
    for instructions starting with the 0xCB, their TWO BYTES
    are considered the opcode, meanwhile for others only the LSB
    is considered the opcode.
    Args:
            instruction (int): Raw instruction

    Returns:
            int: Opcode that can be used to distinguish
            instruction type.
    """
    opcode_lsb = 0x00
    # One before lsb
    possible_opcode_msb = 0x00
    while instruction != 0x00:
        possible_opcode_msb = opcode_lsb
        opcode_lsb = instruction & 0xFF
        instruction >>= 8
    # When the instruction is zero, lsb will be the, well, lsb
    # While the lsb2 will hopefully will be the one before lsb.
    if opcode_lsb == 0xCB:
        return (opcode_lsb << 8) | possible_opcode_msb
    return opcode_lsb


def sort_instructions(instruction: list[dict], resolve_instruction = lambda i: int(i['instruction'], base=16)) -> None:
    """In place sort the instruction according to canonical opcodes.

    Args:
            instruction (list[dict]): List holding the instructions.
            resolve_instruction (Callable[[dict], int]): a function used to resolve from machine code from instruction
    """
    instruction.sort(key=lambda i: get_opcode(resolve_instruction(i)))


def get_tokens_except(except_=[]) -> list[dict]:
    """Get all tokens except those whose opcodes are listed.

    Args:
        except_ (list, optional): Opcodes of the tokens to be ignored.. Defaults to [].

    Returns:
        list[dict]: A list of all tokens besides those listed.
    """
    with open("tokens.json") as file:
        data = load(file)
        tokens: list[dict] = data["tokens"]
    before = len(tokens)
    tokens = [
        token
        for token in tokens
        if get_opcode(token["token"]["original_machine_code"]) not in except_
    ]
    after = len(tokens)
    print(f"Removed {before - after} existing tokens.")
    return tokens


def gen_register(register_name: str) -> dict:
    """Generate a register payload that suits BeemuParam

    Args:
        register_name (str): Name of the register in all caps.

    Returns:
        dict: the BeemuParam as a dictionary.
    """
    return {
        "pointer": register_name == "HL",
        "type": (
            "BEEMU_PARAM_TYPE_REGISTER_16"
            if register_name == "HL"
            else "BEEMU_PARAM_TYPE_REGISTER_8"
        ),
        "value": (
            {"register_16": f"BEEMU_REGISTER_{register_name}"}
            if register_name == "HL"
            else {"register_8": f"BEEMU_REGISTER_{register_name}"}
        ),
    }


def gen_register_16(register_name: str, pointer: bool = False) -> dict:
    """Generate a 16 bit register parameter

    Args:
        register_name (str): Name of the register
        pointer (bool, optional): If set to true, this is a pointer param.

    Returns:
        dict: BeemuParam encoded in json.
    """
    return {
        "pointer": pointer,
        "type": "BEEMU_PARAM_TYPE_REGISTER_16",
        "value": {"register_16": f"BEEMU_REGISTER_{register_name}"},
    }
