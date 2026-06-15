# Dev diary
Rules:
  - Keep it simple
  - Keep it short
  - Keep it focused
  - No AI text generation

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

## June 9 
Turns out i was right, the array lives on the stack and it is static, while the ptr is assigned on the heap and it is dynamic and can change in runtime, but slower. I already knew this but i had forgotten about it.

Wouldn't it be cool if Tig managed at compile time all resource allocations? we already have defer, but something that injected free() stmts where they should at comptime?
You would be making code that looks unsafe but Tig free()'s memory under the hood. But we could stick with defer for now. 


## June 10
I discovered a bug, in sync functions, passing queues/stacks as arguments does not work. It only seems to work with async functions.
Probably today i will have internet again, hope so. 

I found what was wrong with fgetc(), the problem was that both the wrapper and the C function were called the same, and it caused infinite recursion glitch.
I renamed it to filegetc() and it seems to work now. 

## June 11

I managed to sketch and implement (still buggy) an error system, i tried to make it intuitive and easy to use. 
Now i am going to test it.
Also i am going to tackle as much as i can from the spec1.3.2.md file and do as many todo's as possibe before my free Cascade Code runs out. I could do it without it, but it would take me more time and I am afraid of breaking something. I have git undo for git, which works wonders, but still.

I am also thinking when it would be nice to start the self hosting process, i planned that after 1.4.0alpha would be Ok, the rest of the features of beta could await. 

For the self hosting process i am going to rewrite Tig by hand, i could use AI for debuging and error hunting but not for codegen, the reason is both pragmatic and by ideals: By ideals i mean it would be dishonourable. and by pragmatic i mean: no AI has Tig in its training data, this repo's AI could code based on the knowledge it has of the codebase, but i've tried that for non trivial programs and it keeps thinking it is Rust or Zig, it would use `mut`, `var` `*` for ptrs and deref, so i cant use AI to self host the compiler (at least for codegen). I can use it to build the C core, but everything after that must be handmade. 

I had an idea, related to the thing i talked days before
> Wouldn't it be cool if Tig managed at compile time all resource allocations? we already have defer, but something that injected free() stmts where they should at comptime? You would be making code that looks unsafe but Tig free()'s memory under the hood. But we could stick with defer for now. 

What if there was a comptime garbage collection? something that would free memory at compile time? that would be really cool. Something that tracked lifetimes (like Rust's BC but simpler), no runtime overhead. Like a memory linter, it could be called that "memory linter"
The problem comes with input defined at runtime, like user input, files, network data, etc. In that case we would need to allocate at runtime, but we could still do comptime GC for the rest of the program.

Is It Possible?
Theoretically, yes for a subset of programs. For all programs? That's the halting problem.

But Tig could do it for safe subset and require defer for complex cases. That would still be revolutionary.

## June 12

I've worked on a lot of things today and I forgot to write in this file. I am going to try to remember what i did and write it here.
- Fixed queues and stacks not working when allocated
- Added more methods to queues and stacks (size, clear, isEmpty)
- Made Tig catch when we pass to few args or to many args to a function
- Added inline braceless defer statements for ergonomics
- Made inline C work in the global scope
- Made extern fn calls work with old and new syntax
- Fixed defer not working
- Added a dedicated module to the stdlib to manage queues and stacks
- Fixed IO not working, specifically reading files
- Implemented stack and queue primitives
- Implemented basic stack and pointer features
- Implemented async functions
- Added explicit casting
- Fixed Tig compiler asking for ";" when it is optional
- Fixed emitter to handle async with pointer arguments
Some might not be true or mixed memories. 

There is a feature from 1.4 that i really want to implement in 1.3.2, that is passing flags for the C compiler using --
I also thought that, when i make the Tig build system, or the Tig equivalent of Make, it could have a flag to pass flags to the C compiler. it could look like this
```bash
tigre build -- -O2 "-I/usr/local/include -L/usr/local/lib"
```
```bash
tigre run -- -O2 -I/usr/local/include -L/usr/local/lib
```

WIth just the compiler would be

```bash
tigc source.tc -c app -- -O2 -lraylib -L/usr/lib
```

I thought there would be conflict but tigre can just use -- to separate the flags for the C compiler from the flags for tigre.

Adding this would enable tig and tigre to link other clibs 

I was also thinking on this
`tigc a.tc b.tc -c app` 

the rules 
  - Only one file can have main function, linker error otherwise
  - Other files are treated as libs
  - Compiler merges ASTs from all files
  - main from whichever file has it becomes the entry point.

## June 13

Did not code much today, just wrote this diary and thought about the future of Tig.

## June 14

I fixed the if stmt not working on try blocks. Now throw works as intended. 
I am thinking on not going for 1.4.0 at all, but instead make a final 1.3.3 version with just enough stable features to start the self hosting (writing Tig in Tig) process. 

I might not add C codegen at comptime, or comptime, or maybe yes, but I am most interested in self hosting. 

I am thinking on writing a program in Tig to manage this diary. 

Also, select stmts are not working properly, but it is something i can defer for later. 

### on adding generics
So far queues and stacks are hardcoded into the language, but i think it would be better to have a generic implementation that can be used for any type. We need actual generics to make the languge "modable" so future users can make their own modules and data structures without needing to modify or hardcode them into the Compiler core. 

Given Tig's philosophy (minimal, C-transpiled, explicit), I recommend C FFI + Library with a hybrid approach:

Keep queue/stack as built in special cases, they're fundamental and used throughout the stdlib  
Everything else (maps, vectors, lists) as C libraries with ergonomic wrappers
Add a simple macro system only if needed for common patterns
This way:

No compiler changes needed for new containers
Users can leverage existing C libraries
Language stays minimal and focused
Self-hosting compiler can use these libraries without modification