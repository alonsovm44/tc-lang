# v1.4.0-alpha

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
        int lookup_table[] = {%s};
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
            int add_%d(int a, int b) {
                return a + b + %d;
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
