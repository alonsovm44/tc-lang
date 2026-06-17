# v1.4.0
This is the last major update before beggining self hosting phase.

## C++ backend [DEBATIBLE][NON-DECIDED-YET]
Keep the stage0 core in C, but Tig outputs C++17
Migrate the C backend to a C++ backend in order to better support the new features and provide a more robust compilation process.

## Stack and Queue update
Currently we have basic stacks and queue implementation

To be added:

- Fixed size stacks and queues
```
// example
stack<i32> s[10] = {}
queue<f32> q[5] = {} // a queue of 5  

// multiple types
stack<i32, f32, char> s2[10] = {} // this stack accepts integers, floats and characters only

// wildcard

stack<*> s3[10] = {} // this stack accepts any type

// stack of stacks
stack<stack<i32>> s4[5] = {} // this stack accepts stacks of integers only

Same applies to queues.
```

Bad:
    stack<*, i32> s = {} // this is not allowed, since * already includes i32

This is very, `non-explicit` and not following Tig's historic philosophy. But is pragmatic, and my goal is to be pragmatic 


## Collections
Collections are a mix of: Mixins, Generics and Namespaces.
We add a % ions system to define behavior for types. And to unify namespaces and generics.

Currently in Tig we can import other functions from other files via raw immports

```tc
/// lib.tc 

pub fn void foo: {}

/// lib2.tc
pub fn void foo: {}

/// main.tc
@use "lib.tc"
@use "lib2.tc"
fn void main: {
    foo() // conflict - which foo?
}
```
For using libs made by other people, this gets messy fast.

Solution
Use % ions to classify code
```
/// lib.tc
%lib {
    fn void foo: {}
}

/// lib2.tc
%lib2 {
    fn void foo: {}
}

/// main.tc
@use "lib.tc"
@use "lib2.tc"

fn void main: {
    lib.foo() // we call the first %'s foo function
    lib2.foo() // we call the second %'s foo function
}


```

% ions can be genericly parametric and inherit via `:`

```
% balls(T){
    fn void kick: T n {
        print("Kicking the ball")
        if (typeof(T)=="i32"){
            printf("Kicked %d times", n)
        }
        else {
            printf("Error, expected i32, got %s", typeof(T))
        }
    }
}

%foo(T) : balls(i32){
    fn void kick {
        balls::kick(i32, 5)
    }
}

fn i32 main(foo(i32)): {
    foo.kick()
    ret 0
}
```

Example spec of % defs and uses:
```tig
/// Tig 1.3.2 patch 
/// %s

# macro(T){ print(T)} // this is a Macro, text replacement codegen, compile-time, fully implemented, works now.

%myColl(T){ // This is a % ion. It is called myColl, they can have parameters.
// because it % s functions and other items that can be used to group and define behavior
// it is like an inline library
    pin f32 PI = 3.14

    # farwell {print("Farwell!")}

    fn void run(T, ->T data){
        // run code
        if(typeof(->T)=="->i8"){
            print("This is a string")
        }
        else{
            printf("This is a pointer to: %d", T)
        }
    }
    fn void greet(){
        print("Hello from my%")
    }
    async fn doWork(): ->T data, ->queue<T> res {
        ->data = ->data * 2
        res.>push(->data) // since async functions cannot return values, we use a queue/channel to pass the result
        
    }
    fn void sayGoodBye: {
        farwell // calls the macro and expands in comptime
    }
}

// Use

fn void foo(my%(i32)): ->i32 data { // if %(_) wildcard was used, the type must be specified in each call
    macro("Hello") // we call the macro, it expands in comptime to print("Hello")
    my%.run(*, ->data) // we use * to indicate that the type is inferred from the parameter signature at the top
    my%.greet()
    queue<i32> res = {}
    my%.doWork(*, ->data, ->res)
    my%.sayGoodBye()
}
/// %s inheritance ------a---------------------------------------------

%balls(T){
    fn void kick: T n {
        print("Kicking the ball")
        if (typeof(T)=="i32"){
            printf("Kicked %d times", n)
        }
        else {
            printf("Error, expected i32, got %s", typeof(T))
        }
    }
}

%foo(T) : balls(i32){
    fn void kick {
        balls::kick(i32, 5)
    }
}

fn i32 main(foo(i32)): {
    foo.kick()
    ret 0
}

/// Another example -------------------------------------------------------------------

% parent {
    fn void hello {
        print("Hello from parent")
    }
}

% child : parent {
    fn void goodbye {
        print("Goodbye from child")
    }
}

fn void main(child): void {
    child.hello()    // From parent
    child.goodbye()  // From child
}

/// Another example

// ===== std/%ss.tc =====

// Simple %s
% printer {
    fn void print_str(str s) {
        print(s)
    }
    
    fn void print_int(i32 n) {
        printi(n)
    }
}

// %s with state
% counter {
    pin i32 count = 0
    
    fn void increment {
        count = count + 1
    }
    
    fn i32 get_count {
        ret count
    }
}

// %s with type parameter
%stack(T) {
    queue<T> items = {}
    
    fn void push(T item) {
        items.push(item)
    }
    
    fn T pop {
        ret items.pop()
    }
    
    fn bool is_empty {
        ret items.is_empty()
    }
}

// %s with macro
%logger(level) {
    # log(msg) {
        print("[" + level + "] " + msg)
    }
    
    fn void info(->i8 msg) {
        log(msg)
    }
    
    fn void error(->i8 msg) {
        log("ERROR: " + msg)
    }
}

// %s composition
%debug_logger(level) {
    printer  // Include printer %s
    
    # log(msg) {
        print("[" + level + "] " + msg)
    }
    
    fn void debug(->i8 msg) {
        log(msg)
        print_str("Debug: " + msg)
    }
}

// ===== main.tc =====

// Use %ss
fn void main(printer, counter, stack(i32), logger("INFO"), debug_logger("DEBUG")): void {
    // printer %s
    print_str("Hello")
    print_int(42)
    
    // counter %s
    counter.increment()
    counter.increment()
    print_int(counter.get_count())  // 2
    
    // stack %s
    stack.push(10)
    stack.push(20)
    print_int(stack.pop())  // 20
    
    // logger %s
    logger.info("Application started")
    logger.error("Something failed")
    
    // debug_logger %s (includes printer)
    debug_logger.debug("Debugging...")
    debug_logger.print_str("Hello from debug logger")
}

/// Currently in Tig we can import other functions from other files via raw immports

/// lib.tc 

fn void foo: {}

/// main.tc
@use "lib.tc"
@use "lib2.tc"
fn void main: {
    foo()
}

/*
but what if there is another function named foo in lib2.tc?
To avoid collisions, we need to use a different approach.
We can declare functions inside %s, like this:
*/
/// lib.tc
%lib {
    fn void foo: {}
}

/// lib2.tc
%lib2 {
    fn void foo: {}
}

/// main.tc
@use "lib.tc"
@use "lib2.tc"

fn void main: {
    lib.foo() // we call the first %'s foo function
    lib2.foo() // we call the second %'s foo function
}


/// mathutils.tc
%arith(T) {
    fn T add: T a, T b {
        if(typeof(T)=="i32"){
            a = (i32)a 
            b = (i32)b

            ret a + b
        }else 
        {
            ret (f64)a + (f64)b // assumed to be float/double
        }

    }
}

/// main.tc
use "std/mathutils.tc"

fn void main(arith(_)): =>->i8 args { //_ is a wildcard that says (we could use any type here, using arith(i32) would tell the compiler (only i32 types here))
i32 a = 2
i32 b = 3

i32 result = arith(i32).add(a, b) // 
f64 result2 = arith(f64).add(2.5, 3.5) // no collision possible since we carry the namespace 
printi(result)
printf("%f\n", result2)

}

//// %s serve thre purposes

%lib {           // 1. NAMESPACE (organizes code)
    fn foo() {}   //    No collisions with other libs
}

%arith(T) {      // 2. GENERICS (type-parameterized)
    fn add() {}   //    Works with i32, f64, etc.
}

%logger :: parent {     // 3. MIXINS (behavior injection)
    fn log() {}   //    Can be added to functions
}

// Struns and %s

% col {
    strun Point:{
        i32 x,
        i32 y
 
        fn void print: // method
        {
            printi(x)
            printi(y)
        }
    }

}

% col2 : col {
    // inherits Point strun
    fn void foo: ->Point p{
        p.>x = 30
        p.>y = 40
        p.>print() // pointer access to strun method

    }
}
fn void main(col,col2): {
    col.Point p = {10, 20} // we instantiate a point
    p.print()
    col2.foo(&p)

}

```



# Keywords in 1.4.0
- **19 keywords** — 
`if`, 
`loop`, 
`break`, 
`defer`, 
`ret`, 
`strun`, 
`fn`, 
`use`, 
`private`, 
`pin`, 
`match`, 
`else`, 
`enum`, 
`async`, 
`select`,
`throw`,
`try`,
`catch`,
`comptime`,
`% `