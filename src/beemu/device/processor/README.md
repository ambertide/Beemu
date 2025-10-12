# Processor Modal

General flow of the instructions goes thus:

* `processor` reads a single instruction.
* `tokenizer` takes this raw hexadecimal instruction
and outputs one OR more instructions, this is because
certain Gameboy instructions may modify registers
before load/store and such.
* These instructions are passed into the `executor`
which modifies the `memory` and the `registers`,
thus modifying the machine state.
* The `processor` than adds the clock cycles.