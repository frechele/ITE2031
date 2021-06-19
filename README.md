# ITE2031 - Computer Architecture Project (LC-2K)
In this project, we implemented simple assembler and simulator for LC-2K Instruction-Set Architecture.

## What is LC-2K?
The LC-2K is an 8-register, 32-bit computer. All addresses are word-addresses. The LC-2K has 65536 words of memory.

## Supported instructions
This simulator supportes 3 type instructions followed.

- R-type instructions (add, nor):
  - bits 24-22: opcode
  - bits 21-19: reg A
  - bits 18-16: reg B
  - bits 15-3: unused (should all be 0)
  - bits 2-0: desgReg
- I-type instructions (lw, sw, beq):
  - bits 24-22: opcode
  - bits 21-19: reg A
  - bits 18-16: reg B
  - bits 15-0: offsetField (a 16-bit, 2's complement number with a range of -32768 to 32767)
- J-type instructions (jalr):
  - bits 24-22: opcode
  - bits 21-19: reg A
  - bits 18-16: reg B
  - bits 15-0: unused (should all be 0)
- O-type instructions (halt, noop):
  - bits 24-22: opcode
  - bits 21-0: unused (should all be 0)

## How to build
First of all, you should clone this repository.
```bash
git clone https://github.com/JYPark09/ITE2031
```

And by using make, you can get assemble and simulate programs.
```bash
cd ITE2031
make
```

## Test cases
### samples/test.as
This test performs the same behavior as the C code below.

```c++
int i = 5;

do {
    i = i + (-1);
} while (i != 0);
```

### samples/test1.as
This test performs the same behavior as the C code below.
```c++
int i;
int res = 0;

for (i = 1; i < 10; ++i)
    res += i;
```