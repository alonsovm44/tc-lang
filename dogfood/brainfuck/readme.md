# a brainfuck interpreter in Tig
Source is in `main.tc`

You need Tig 1.3.1 to compile this script. 
Look at the repo root to see how to compile Tig

Compile with
```bash
./tigc /dogfood/brainfuck/main2.tc -c bf 
```
Add `-o` to look at the C source
```bash
./tigc /dogfood/brainfuck/main2.tc -o bfc.c
```

Run scripts with
```sh
./bf script.bf
```

## Bf rules
|+ |increases current cell value by 1, wraps if overflow of 255
|- |decreases current cell value by 1, wraps if underflow of 255
|< |moves pointer "left" (or minus), blocks if bottom
|> |moves pointer "right" or plus, blocks if top
|. |prints current cell ascii value in the stdout
|, |asks for a single char input in the stdin 
|[ |Opens a loop,
| ]|Closes the loop

- Loop logic
If current cell value is not 0, continue executing the code inside [] 

Algorithm

1. open the .bf file
2. store the code in a 
