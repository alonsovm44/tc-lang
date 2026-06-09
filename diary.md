# Dev diary

## June 4 2026:
For the rest of the day i wont add new features to Tig, only focus on making more documentation. A lang is as good as its documentation. (or not)

## June 5 2026:
I ran out of AI, i will use the time left i have with internet to document edge cases and errors in Tig.
Someone gave me the idea of using public WiFi in libraries. I didn't think about it and it would work for when I no longer have connection at home. 

Queues were not working for some reason, i patched it. 

## June 6 

I should work on a package manager for Tig or smth. 

I am planning to write a simple dev diary CLI tool in Tig to keep track of ideas, instead of doing it manually. I actually already have a tool called diaryx but it is buggy and i think i deleted it a while back

I will focus in documenting Tig.

To update the docs, i might use Gendox, i built it for that. But i dont have the API key. 

## June 8
I have no internet but managed to sketch a simple interpreter in Tig, this has been useful as i've been able to test 1.3.0 - 1.3.1 features and make a more comprehensive todo list with things that need to be done plus ideas. 

Yesterday( actually it was today i think ) I used gendox to make docs for the codebase, but used Ollama for it, so the docs are rather raquitique.

For some reason while testing async funcntions, i tested without including the runtime `async.tc/async.h`and it compiled and ran, 
and for some obscure reason now it cant find the lib, even if it is where it should. 
```
PS C:\Users\diego\.projects\langs\tig\tc> ./tigc tests/async/owns.tc -c owns -o tests/async/owns_1.c
  gcc "tests/async/owns_1.c" -std=c11 -lm -o "owns" "compiler/src/runtime.o"
tests/async/owns_1.c:10:10: fatal error: stdlib/async.h: No such file or directory                                                                                                                                                                         
   10 | #include "stdlib/async.h"                                                                                                                                                                                                                          
      |          ^~~~~~~~~~~~~~~~                                                                                                                                                                                                                          
compilation terminated.
```
even inlinling does not work

```
PS C:\Users\diego\.projects\langs\tig\tc> ./tigc tests/async/owns.tc -c owns -o tests/async/owns_1.c
error[E009]: cannot open file 'tests/async/stdlib/async.tc'
 --> tests/async/owns.tc:3:1
   |
 3 | @use "stdlib/async.tc"
   | ^^^^ cannot open file 'tests/async/stdlib/async.tc'

E009
```
It seems like the file is corrupt?

Now that i see, the .tc file has syntax not of Tig, but a mixin of C like function ptrs. The AI seems to have hallucinated it. 

I'll have to wait for internet to come back to debug and test async not working, i need money to buy more cascade code licence, but i am unemployed and almost
dropping out of college for the third time. I wish someone funded me. 

### On the difference between ptr alloc and []
I was wondering why you can use a ptr like an array,
```
->i32 ptr = alloc(i32, 8)
ptr[1] = 1

//vs
i32[8] arr = {}
arr[1] = 1
```
What is the difference?

Maybe the array lives on the stack and is auto managed while the other lives on the heap, it is probably the explaination. Because arrays dont need free memory statements. 

I think i get it, the array has its memory allocated in continuous memory spaces, maybe ptrs don't. I need to look it up. 

I recompiled the async test and it compiled and ran well, for some reason. It seems like it was a heisenbug. 

I ran it again, when i compile it `-c` only it compiles ok, but when I transpile `-o` it does not work.
