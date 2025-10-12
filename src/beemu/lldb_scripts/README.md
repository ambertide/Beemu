# What the f*ck

`beemu` was originally developed on a small
Macbook Air (M2), and `lldb` was used heavily
for its debugging, this directory includes
debugging scripts written in python for `lldb`.

These are not used in build time.

## How the f*ck do I run this??

For example, you can run the `emitted_bytecode`
by:

```fish
PYTHONPATH=$(lldb -P) /usr/bin/python3 src/beemu/lldb_scripts/emitted_bytecode.py 0x35
```

This is for `fish`, but it should work with
`zsh` and `bash` if you replace the substituation
syntax with the correct version.
