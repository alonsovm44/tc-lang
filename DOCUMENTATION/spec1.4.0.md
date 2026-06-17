# v1.4.0
This is the last major update before beggining self hosting phase.

## C++ backend
Migrate the C backend to a C++ backend in order to better support the new features and provide a more robust compilation process.

## Comptime execution
Following the "explicit" nature of the language, comptime execution is explicit and requires the `comptime` keyword.
```tcs
comptime {
    // Code that executes at compile time
}
```

Example
```tc
fn void make_array(int size) {
    comptime {
        "C"{
            // Code that executes at compile time
            int[] arr = new int[size];
            for (int i = 0; i < size; i++) {
                arr[i] = i;
            }
        }
    }
}
// or this

comptime {
    // Imagine this reads a config file or a JSON string
    ->u8 data = "10,20,30,40"
    
    // Generate a C array initialization
    "C"{
        int lookup_table[] = {collections};
    }
}

fn void main: {
    // lookup_table is now a real C array in your binary
    printi(lookup_table[1]) // Prints 20
}

// other example

fn void generate_adders: {
    i32 i = 0
    loop if(i<=100){
        // generate functions at comptime
        "C"{
            int add_collectiond(int a, int b) {
                return a + b + collectiond;
            }
        }
        i++
    }
}

comptime {
    generate_adders()
}

fn void main:{
    printi(add_99(1, 2)) // Prints 102
}
```


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
We add a collections system to define behavior for types. And to unify namespaces and generics.

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
Use collections to classify code
```
/// lib.tc
collection lib {
    fn void foo: {}
}

/// lib2.tc
collection lib2 {
    fn void foo: {}
}

/// main.tc
@use "lib.tc"
@use "lib2.tc"

fn void main: {
    lib.foo() // we call the first collection's foo function
    lib2.foo() // we call the second collection's foo function
}


```

Collections can be genericly parametric and inherit via `:`

```
collection balls(T){
    fn void kick: T n {
        print("Kicking the ball")
        if (typeof(T)=="i32"){
            printf("Kicked collectiond times", n)
        }
        else {
            printf("Error, expected i32, got collections", typeof(T))
        }
    }
}

collection foo(T) : balls(i32){
    fn void kick {
        balls::kick(i32, 5)
    }
}

fn i32 main(foo(i32)): {
    foo.kick()
    ret 0
}
```

Example spec of collection defs and uses:
```tig
/// Tig 1.3.2 patch 
/// Collections

# macro(T){ print(T)} // this is a Macro, text replacement codegen, compile-time, fully implemented, works now.

collection myCollection(T){ // This is a collection. It is called collection, they can have parameters.
// because it collects functions and other items that can be used to group and define behavior
// it is like an inline library
    pin f32 PI = 3.14

    # farwell {print("Farwell!")}

    fn void run(T, ->T data){
        // run code
        if(typeof(->T)=="->i8"){
            print("This is a string")
        }
        else{
            printf("This is a pointer to: collectiond", T)
        }
    }
    fn void greet(){
        print("Hello from myCollection")
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

fn void foo(myCollection(i32)): ->i32 data { // if collection(_) wildcard was used, the type must be specified in each call
    macro("Hello") // we call the macro, it expands in comptime to print("Hello")
    myCollection.run(*, ->data) // we use * to indicate that the type is inferred from the parameter signature at the top
    myCollection.greet()
    queue<i32> res = {}
    myCollection.doWork(*, ->data, ->res)
    myCollection.sayGoodBye()
}
/// collections inheritance ------a---------------------------------------------

collection balls(T){
    fn void kick: T n {
        print("Kicking the ball")
        if (typeof(T)=="i32"){
            printf("Kicked collectiond times", n)
        }
        else {
            printf("Error, expected i32, got collections", typeof(T))
        }
    }
}

collection foo(T) : balls(i32){
    fn void kick {
        balls::kick(i32, 5)
    }
}

fn i32 main(foo(i32)): {
    foo.kick()
    ret 0
}

/// Another example -------------------------------------------------------------------

collection parent {
    fn void hello {
        print("Hello from parent")
    }
}

collection child : parent {
    fn void goodbye {
        print("Goodbye from child")
    }
}

fn void main(child): void {
    child.hello()    // From parent
    child.goodbye()  // From child
}

/// Another example

// ===== std/collectionss.tc =====

// Simple collections
collection printer {
    fn void print_str(str s) {
        print(s)
    }
    
    fn void print_int(i32 n) {
        printi(n)
    }
}

// collections with state
collection counter {
    pin i32 count = 0
    
    fn void increment {
        count = count + 1
    }
    
    fn i32 get_count {
        ret count
    }
}

// collections with type parameter
collection stack(T) {
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

// collections with macro
collection logger(level) {
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

// collections composition
collection debug_logger(level) {
    printer  // Include printer collections
    
    # log(msg) {
        print("[" + level + "] " + msg)
    }
    
    fn void debug(->i8 msg) {
        log(msg)
        print_str("Debug: " + msg)
    }
}

// ===== main.tc =====

// Use collectionss
fn void main(printer, counter, stack(i32), logger("INFO"), debug_logger("DEBUG")): void {
    // printer collections
    print_str("Hello")
    print_int(42)
    
    // counter collections
    counter.increment()
    counter.increment()
    print_int(counter.get_count())  // 2
    
    // stack collections
    stack.push(10)
    stack.push(20)
    print_int(stack.pop())  // 20
    
    // logger collections
    logger.info("Application started")
    logger.error("Something failed")
    
    // debug_logger collections (includes printer)
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
We can declare functions inside collections, like this:
*/
/// lib.tc
collection lib {
    fn void foo: {}
}

/// lib2.tc
collection lib2 {
    fn void foo: {}
}

/// main.tc
@use "lib.tc"
@use "lib2.tc"

fn void main: {
    lib.foo() // we call the first collection's foo function
    lib2.foo() // we call the second collection's foo function
}


/// mathutils.tc
collection arith(T) {
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
printf("collectionf\n", result2)

}

//// collections serve thre purposes

collection lib {           // 1. NAMESPACE (organizes code)
    fn foo() {}   //    No collisions with other libs
}

collection arith(T) {      // 2. GENERICS (type-parameterized)
    fn add() {}   //    Works with i32, f64, etc.
}

collection logger :: parent {     // 3. MIXINS (behavior injection)
    fn log() {}   //    Can be added to functions
}

// Struns and collections

collection col {
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

collection col2 : col {
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

## Type inference
Use * to infer type

```tc
fn void main: {
    *x = 10 // x is inferred as i32
    *y = 3.14 // y is inferred as f64
    *z = "hello" // z is inferred as string
}
```

Example uses
```
// v1.3.2 features
use "stdlib/io.tc"

fn void main: {
    *x = 10 // * means infer type, like C++ auto
    
    if(typeof(x)=="i32"){
        printi(x)
    }else{
        printf("collectiond", x)
    }
}

fn i32 foo(io): *data{
    data = io.read_data()
    ret data
}

// return type inferred

fn *myFunc(collection): i32 x{
    x = collection.get(0)
    ret x // return type inferred from x
}

// using both

fn *myFunc2(coll): *x {
    x = coll.get(0) // type of x inferred from called return value
    ret x // return type inferred from x
}

/// inference with type constrains

collection numeric(T) {
    fn T add: T a, T b {
        ret a + b
    }
}

fn void main(numeric(_)): {
    *x = numeric(i32).add(5, 10)   // i32
    *y = numeric(f64).add(3.14, 2.0) // f64
    *z = numeric(->i8).add("a", "b") // Would be str if supported
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
`collection`