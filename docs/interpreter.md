# Interpreter

Interpreter consists of two parts, parser and
invoker.

## Parser 

After an instruction is converted into a token,
or more formally a `BeemuInstruction`, that
instructions must now be converted into a
`BeemuCommandQueue`.

Each member of the `BeemuCommandQueue` is a
`BeemuMachineCommand`, and specifies either

* a value  that will be written into a specific 
place in the machine state. a `BeemuWriteOrder`
* a `HALT` order. (`BeemuHaltOrder`)

Parser takes the machine state and generates
a `BeemuCommandQueue` object.

## Invoker

Invoker in turns takes the queue and starts
executing the commands one by one, until such
time it hits a `HALT` command, it then stops
and waits for the next cycle.