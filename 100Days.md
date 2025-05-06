# On Beemu's Next 100 Days.

On May 10th, I will be leaving for my compulsory military service for a month.
This leaves Beemu's tokenizer rewrite incomplete and I am leaving my thoughts here
so that I may pick up where I left.

Some of these may sound cryptic as I am just trying to wrap this up, future me,
I am sorry.

## On the Tokenizer Rewrite, or 45 Days after now.

442 out of 526 insturctions are now tokenized, remaining instructions fall into one
of the following categories:

* LOAD 16 instructions: Almost complete, `POP`, `PUSH` and `PEEK` (`$F8`) can be
implemented as `LOAD r16, [SP]` (or reversed) with `postLoadOperation`.
`PEEK` requires some additions to `BeemuPayload` to support `SP + 8` semantics without
changing `SP`, perhaps changing `BeemuPostLoadOperation` will suffice.
* ARITHMATIC 16 instructions: This one is extremely straightforward, unlesss I am
missing something
* System Instructions, also straightfowards
* Control flow instructions like `JR`, `CALL` and `RET` should be tokenized to their
maximum `duration_in_clock_cycles` for their conditional variants, for the maximum can
be cut short, but the minimum cannot.

## On the Stack Machine

A problem arises when we realize that clock cycles must be controlled, instructions
must be able to halt execution etc etc, this is complicated, here is how we will solve it:

* Instructions will be simplified to a stack machine, which will ONLY have the following
commands available to it:
	** `POP TARGET`
	** `PUSH TARGET`
	** Arithmatics such as `AND`, `OR`, `CMP`, `XOR`, `SUB`, and possibly their 16 bit variants
* It is possible we will need two stacks, one for `uint8_t` and one for `uint16_t`,
it is also possible to make it a `void*` stack perhaps but that just sounds like a horrible
idea, one option is to make a `struct StackPayload` perhaps with a tagged union? again
more research is necessary.

BUT ANYWAY

* The main point is that the tokens can be simplified into stack commands, so we will have:
`tokenizer -> parser -> executor` flow going on, we can than use the `POP` and `PUSH` 
instructions to write to, or from, memory, or flags, or register, etc etc. (this may mess
up cache locality, possibly, or maybe not, my mac wil close soon so).
* ANYWAY, parses will simplify every token to a bunch of commands, which the executor will
execute on its stack using a queue, (stack for values queue for commands etc), and the
executor may only make a single memory read/write/whatever, which gives us the timing,
VOILA.

```c
struct BeemuStackCommand {
	// This can also be stuff like BREAK, or HALT maybe.
	enum BeemuStackCommandVerb command;
	enum BeemuStackCommandPayloadType payload_type;
	union {
		uint8_t *addr8;
		uint16_t *addr16;
		uint8_t value8;
		uint16_t value16;
		BeemuRegister register_;
		void* nop = 0;
	} payload;
}
```
