# Generate the example machine states to be used in testing.

from itertools import cycle, islice
from json import dump

machine_states = {
    "default": {
        "registers": {
            # A, B, C, D, E, H and L respectively.
            "registers": [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02],
            # Flags, the last four bits are useless, if I am able to
            # read what I wrote.
            "flags": 0b11110000,
            "stack_pointer": 0xFF,
            "program_counter": 0x00
        },
        "memory": {
            "memory_size": 2**16 - 1,
            # Fill with numbers repeating from 0x00 to 0xFF until addressable space (0xFFFF) is over.
            "memory": [mem_value for mem_value in islice(cycle(range(0x00, 0xFF + 1)), 0xFFFF + 1)]
        },
        "processor_state": "BEEMU_DEVICE_NORMAL",
        "interrupts_enabled": True,
        "elapsed_clock_cycle": 0
    },
    # this is just a machine with large number of stack values.
    "highstack": {
        "registers": {
            # A, B, C, D, E, H and L respectively.
            "registers": [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02],
            # Flags, the last four bits are useless, if I am able to
            # read what I wrote.
            "flags": 0b11110000,
            "stack_pointer": 0xBBFF,
            "program_counter": 0x00
        },
        "memory": {
            "memory_size": 2**16 - 1,
            # Fill with numbers repeating from 0x00 to 0xFF until addressable space (0xFFFF) is over.
            "memory": [mem_value for mem_value in islice(cycle(range(0x00, 0xFF + 1)), 0xFFFF + 1)]
        },
        "processor_state": "BEEMU_DEVICE_NORMAL",
        "interrupts_enabled": True,
        "elapsed_clock_cycle": 0
    },
    # Not Zero, Not Carry
    "nznc": {
        "registers": {
            # A, B, C, D, E, H and L respectively.
            "registers": [0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x01, 0x02],
            # Flags, the last four bits are useless, if I am able to
            # read what I wrote.
            "flags": 0b01100000,
            "stack_pointer": 0xBBFF,
            "program_counter": 0x00
        },
        "memory": {
            "memory_size": 2**16 - 1,
            # Fill with numbers repeating from 0x00 to 0xFF until addressable space (0xFFFF) is over.
            "memory": [mem_value for mem_value in islice(cycle(range(0x00, 0xFF + 1)), 0xFFFF + 1)]
        },
        "processor_state": "BEEMU_DEVICE_NORMAL",
        "interrupts_enabled": True,
        "elapsed_clock_cycle": 0
    }
}

if __name__ == '__main__':
    output = {
        "processors": []
    }

    for machine_state_name, machine_state in machine_states.items():
        output['processors'].append({
            "preset": machine_state_name,
            "processor": machine_state
        })

    with open('processors.json', 'w') as file:
        dump(output, file, indent='\t')