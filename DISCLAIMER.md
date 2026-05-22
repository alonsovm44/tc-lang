# Quick FAQ

Q: What is this?
A: This is a programming language/transpiler I'm building as a learning exercise and hobby.

Q: What is the purpose?
A: A general purpose systems language as compact and minimal as possble. Yet capable of anything (not a turing tarpit, but not bloated either)

Q: Why?
A: I wanted to learn more about compiler design and language implementation. To teach myself programming 

Q: Is it memory sage?
A: It does not enforce safety but I added `defer` to help with memory management.

Q: Are you a CS major?
A: No, I am an industrial engineering undergrad, I like CS as a hobby but I want to have a coding job one day.

Q: Is this production ready?
A: Use at your own risk.

Q: Why C backend?
A: It is the mainstram language I know better. Also because I don't know enough of LLVM to write the LLVM codegen, and I think LLVM is overkill and would make the project bloated (although the tradeoff is portability). I could leverage the AI to do it, but I want to know exactly what am I doing, i don't like blackboxes. (Yet I use an AI blackbox, Oh the hypocrisy)

In essence:
I wanted to keep the project tiny with a C transpiler. Nim does it anyway.

Q: Why no shadowing?
A: We follow a "no implicit" philosophy (although we could arrive at some exceptions), i felt like shadowing was a way of making data have multiple meanings in different parts of the code.

Q: Do you know how to code in any language?
A: I have most experience with C and some C++, and i've tinkered with Rust and Python but I am most comfortable around C

Q: Do you use AI?
A: I'll be honest, yes, a lot. But as a speed boost. There is no honor unlike handcoding everything. I am not a very seasoned dev yet. I know it has less merit. But my philosophy is to use modern tools for modern problems.

Q: Was this FAQ Ai generated?
A: No, I hand wrote it

Q: Why the name Tight C?
A: It is a double meaning pun, it abbreviates as tC which is a reference to holy C (the programming language) by Terry Davis (Ct) the t is a cross/crucifix. And it is called Tight since it is meant to be bare bones without decaying into assembly-like.

Q: Why the function syntax is so weird? (v1.0.0)
A: Sorry for that, i'll see if i can make it better for v1.1 onwards

Q: Can I contribute?
A: Yes! I'll make a contributing file soon. But follow the philosophy on spec1.0.0

Q: Function pointers? How I am supposed to make a linked list?
A: I'll add them for 1.1

Q: Where are you from?
A: Mexico