# furth
A forth like, interpreter written in Tig

Demo program
```
1 // pushes 1 to the stack
2 // pushes 2 to the stack
+ // pops last two elements from the stack, adds them, and pushes result to the stack
! // prints value at the top of the stack

```

# the algorithm

1. open the source file
2. make a queue the same length (implicit) as the source file length
3. store each char of the program into the array

// interpetation

4. loop over poping each member of the queue
- if char is an ascii number, get value and push value to the stack 
- if char is + - * or /, pop last two values of the stack, operate them and push result to the top of the stack
- if char is "!" print the char at the top of the stack 