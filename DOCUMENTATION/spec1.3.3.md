# v1.3.3
Runtime patch

- [ ] Make async runtime production grade

## Allocation function Markers, aka Leaky Functions

Sometimes you need owned data to outlive the function where it is defined. You have to remember that the caller must free it. Defer works nice if your data is freed in the same scope, but more often that data is returned to the caller. 

To solve this, we need an annotation that tells the compiler that the function allocates memory that should be freed by the caller. In other words, the function returns an owned type. Idea: use "@" to declare a leaky function
```
@fn ->i8 foo: {
    ->i8 ptr = alloc(i8, 10)
    ret ptr
} 

// caller
fn void main: {
    ->i8 ptr = @foo() // we use the marker to indicate that the function allocates memory that should be freed by the caller, the compiler enforces that any instance of a marker function must be called with "@"
    defer free(ptr)
}

```

If the compiler finds a function with heap allocations and no marker, it should throw a compile error. This applies if the return type is a Tig owned data type. 
1) A raw pointer
2) A fat pointer
3) A strun containing a raw pointer
4) A strun containing a fat pointer
(File handlers and most handlers are raw pointers themselves, so no need to be specific about thems)

Tig types of data (just as a reminder):

Tig v1.3.0 classifies types into three categories with different ownership semantics:

1. **Value Types** (Primitives: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`):
   - Passed by value (implicitly copied) to both sync and async functions
   - No ownership transfer needed - each copy is independent
   - Safe to pass without `@` or `pin`

2. **Reference Types** (Channels: `queue<T> / stack<T>`):
   - Thread-safe concurrent queues/channels with internal synchronization
   - Passing a channel copies its reference handle, not its ownership
   - Safe to pass to multiple async functions without `@` or `pin`

3. **Owned Types** (Pointers: `->T`, `=>T`, and structs containing pointers):
   - Unique resources that must be explicitly managed
   - MUST be moved with `@` (transfer ownership) or shared with `pin` (immutable sharing)
   - This prevents shared mutable state and data races

### Edge case: void functions that modify arguments and allocate memory
fn void alloc_buffer: ->i8 buffer {  
    buffer = alloc(i8, 100)  // The caller provided the buffer pointer
    // buffer is not returned, so ownership stays with caller
}

fn void main: {
    ->i8 buffer
    alloc_buffer(buffer)  // Caller owns buffer
    free(buffer) // but we still have to remember to free it.
}

## Edge case: Internal allocations
fn void internal_alloc: {
    ->i8 buffer
    alloc(buffer)  // Internal allocation, no need to free
    
    // Use buffer...
    free(buffer)
    ret value // if the value is not an owned type, there is no need for "!fn"
    // intenral memory leaks (like allocating and forgeting to free inside a function) can be handled by Tig's memory linter, which is a static analysis tool that checks for memory leaks and other anti-patterns.
}
## Library for hash maps

[ ] - Create a library and generic type for hash maps

```
use "stdlib/map.tc"

fn void main:
{
    map<->i8, i32> m = {{"key", 1}, {"key2", 2}}
    m["key3"] = 3
    m["key4"] = 4

    m.remove("key")
    i32 size = m.length()
    // or can be 
    // i32 size = m.size()
    // and other useful methods
}

```
This is the last built in to be added before 1.4.0

## Select is a stub, implement it
Make select stmt not a stub anymore before moving to self hosting


## My
Implement my to specify which objects are private.
// math.tc
fn i32 add(i32 a, i32 b) {  // PUBLIC by default
    ret a + b
}

my fn void helper(i32 x) {  // PRIVATE to this collection/file
    printi(x)
}

my struct InternalData {    // PRIVATE type
    i32 secret
}

// main.tc
@use "math.tc"

fn void main(): void {
    add(1, 2)        // OK - public
    helper(5)        // ERROR - private!
    InternalData d    // ERROR - private type!
}

### Comptime 
Instead of a comptime keyword we use the # symbol to indicate compile time code.
```tig
# IF (condition){ // special macro 
    // this code gets evaluated at compile time
} # ELSEIF (condition) { // special macro
    // this code gets evaluated at compile time
} # ELSE { // special macro
    // this code gets evaluated at compile time
}

# {
    // this code gets evaluated at compile time
}

#fn void myCompTimeFn: {
    // this code gets evaluated at compile time
}

fn void main:
{
    myCompTimeFn()
}

```

### Include macro for C code

```tig
"C"{
    # include "mylib.h" // direct C code embedded
}

```
Alternative:
Dedicated macro to include C libs
```tig
# use {"mylib.h"}
```

## Function modifiers

<trait> fn(<mod>) <name>: <T><arg>,... {
    // function body
}

function mods:
```
# - comptime fn
@ - allocates, (this function returns an owned type) aka leaky function
! - risky function, it might throw an error, 
? - TBD (to be determined later)
// example uses

#fn void myCompTimeFn: {
    // this code gets evaluated at compile time
}

@fn ->i8 myAllocFn: {
    // this function allocates memory that should be freed by the caller
    ->i8 ptr = alloc(i8, 10)
    ret ptr
}

error myerror: {
    // this is a custom error type
}

!fn void myRiskyFn: ->Data data {
    // this function might throw an error
    if (condition) {
        throw myerror
    }
}

fn i32 main: {
    Data data = get_data()
        !myRiskyFn(&data) { // this works as a compact way of saying
           myerror(data) ret 1,
           othererror(data) ret 1,
            yetanothererror(data) ret 1,
            _ {
            // wildcard handler 
            ret 0
           }
        }
}
// example

fn i32 foo: {
    !function_that_might_throw_error(x){
        // pattern matching block with body of possible errors
        E001() ret return_value,
        E002() ret return_value,
        E003() ret return_value,
        E004() ret return_value,
        _ {
            ret default_value
        }
    }
}
    
This is sugar for

fn i32 foo: {
    try{
        riskyFn(x)
    }catch{
        // pattern matching block with body of possible errors
        E001() ret return_value,
        E002() ret return_value,
        E003() ret return_value,
        E004() ret return_value,
        _ {
            ret default_value
        }
    }
}
```
Using in combination

```
my async !@fn void myRiskyAllocFn: { // my means it is private
    // this function allocates memory that should be freed by the caller
    // and might throw an error
    ->i8 ptr = alloc(i8, 10)
    ret ptr
}

my async !fn void myRiskyFn: {
    // this function might throw an error
    if (condition) {
        throw myerror
    }
}
```

# priorities

HIGH PRIORITY:
1. my keyword implementation
2. Hash map library
3. select statement implementation

MEDIUM PRIORITY:
4. Async runtime fixes
5. Map iteration (for loops over maps)

LOW PRIORITY:
6. Map performance optimizations

# Keywords for 1.3.3

- **19 keywords** — 
`if`, 
`loop`, 
`break`, 
`defer`, 
`ret`, 
`strun`, 
`fn`, 
`use`, 
`pin`, 
`match`, 
`else`, 
`enum`, 
`async`, 
`select`,
`throw`,
`try`,
`catch`,
`raw`,
`my`, (new)