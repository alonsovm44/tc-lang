# v1.3.1

## Try/Catch blocks for error handling
Simple try/catch blocks for error handling, no complex error types or traits.
```tc
try {
    // Code that might throw
    int result = divide(10, 0);
} catch (error) {
    // Handle error
    print("Error: ", error);
}
```

## Comptime execution
Following the "explicit" nature of the language, comptime execution is explicit and requires the `comptime` keyword.
```tc
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