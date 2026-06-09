# a brainfuck interpreter in Tig

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