# Commentaries on 1.4.0 features

The best idea from 1.4.0 is queue based ownership transfer, and using queues (or stacks) to transfer data between threads and scopes

In most languages, moving data between threads, scopes, or memory regions is complex and error-prone. Tig 1.4.0 introduces a unified mechanism: queues (and stacks) become the only way to transfer ownership of data across boundaries.

```tig
{
    // some scope
    queue<i32> ch = {}
    ch.push(@x)  // Give ownership
}
i32 y = ch.pop()  // Take ownership
```

That's it, No borrow checker. No lifetime annotations. No hidden copies. Just explicit transfer.

## For values allocated in an arena that need to outlive it.
How can we make it without leaks?

2. Escaping an Arena
Arenas free all memory on exit. But what if you need a value to live longer?

```tig
queue<i32> persistent = {}

arena[1024] {
    i32 temp = 42
    persistent.push(@temp)  // Transfer OUT of the arena
}  // Arena freed, but 'temp' lives on in the queue

i32 value = persistent.pop()  // Receive it in outer scope
```
The arena gets cleaned up. The value survives. No leaks. No copies

## Thread Communication
Passing data between async threads.

```tig
queue<Image> images = {}

async {
    Image img = load_image("player.png")
    images.push(@img)  // Give to consumer thread
}

async {
    Image img = images.pop()  // Take ownership
    render(img)
}  // img freed when async block exits
```

## Memory safe pattern

The Pattern
```tig
// Pipe setup
queue<Work> input = {}
queue<Result> output = {}

// Producer thread
async {
    Work w = generate_work()
    input.push(@w)  // Pipe data IN
}

// Worker thread  
async {
    Work w = input.pop()    // Receive ownership
    Result r = process(w)   // Transform
    output.push(@r)         // Pipe further
}

// Consumer thread
async {
    Result r = output.pop()  // Take final result
    handle(r)
}
```

## Anti-patterns
Cannot Transfer Twice
```tig
i32 x = 10
q1.push(@x)
q2.push(@x)  // Error: x is already dead
```

Cannot Use After Transfer
```tig
i32 x = 10
q.push(@x)
printi(x)  // Error: cannot access dead variable 'x'
```

Must Transfer Out Before Scope Ends
```tig
{
    i32 x = 10
    q.push(@x)  // Ownership moves to q
}  // ✅ x is dead, but q owns the value
```

Pin Prevents Transfer
```tig
pin i32 x = 10
q.push(@x)  // Error: cannot transfer pinned variable
```

Pinned data is read-only for a reason - it may be shared across multiple scopes.