# v 1.2.1

# enums
[APPROVED]
Simple enums since using match with magic numbers causes bugs

[DONE]

enum Color {
    RED
    GREEN
    BLUE
    BLACK
    WHITE
}

Types default to i32. Note: C doesn't support enum type specification like C++, so explicit types are ignored for now (all enums are int32_t in C).

enum Color {
    RED
    GREEN
    BLUE
    BLACK
    WHITE
}

# if else
using _if and _ is cumbersome
[DONE]

Implemented `else` and `else if` syntax like in Go:

```tc
if (condition) {
    // code
} else if (condition) {
    // code
} else {
    // code
}
```

The `_` wildcard is now reserved for match statements only.

# Inlining
Escape hatch for when you need to use C code, aka TightC incompetence.

Inline C
```c
"C"{
    // C code
    int func(int x){
        return x*x;
    }
}
```
## Inlining rules

When inlining C code, the compiler should check for name conflicts and provide a way to rename functions.
When inlining C code, all inline blocks go at the top of the file. 
Inlined C code can interact with outside defined variables. 

## [possible]
Inline assembly

Adding inline assembly
```s
"asm"{
    mov eax, 12
}

```
Overkill for now
We could considere this later

# Pointer to strun acess
[DONE]
Since -> is reserved for
1. pointer decl
2. dereference

[decision]
Using it for a c style field access
x->field 
would overload and make parser ambiguity
So we use
x.>field 


```tc
strun Person{
    i8 age
    ->i8 name
}
fn void bar: Person p {
    p.name = "Jane"
    p.age = 21
}

fn void foo: ->Person p {
    p.>name = "John"
    p.>age = 23
}

fn void main: {
    Person p
    bar(p)
    foo(&p)
}
```
C equivalent
```c
struct Person {
    int age;
    char* name;
};

void bar(struct Person p){
    p.name = "Jane";
    p.age = 21;
}

void foo(struct Person* p){
    p->name = "John";
    p->age = 23;
}

int main(){
    struct Person p;
    bar(p);
    foo(&p);
    return 0;
}
```

# methods

```
strun Person {
    i8 age
    ->i8 name

    fn void sayHi: {
        print("Hi! my name is ")
        print(name)
    }
    fn i32 getAge: {
        ret self.age
    }
}

Person person
person.name = "John"
person.sayHi() 

```

## Methods rules

Methods cannot have the same name as a field.
Bad:

```tc
strun Foo {
    i32 bar  // field
    
    fn void bar: {  // method
        print("method bar")
    }
}

fn void main: {
    Foo f
    f.bar()      // method
    f.bar = 42   // field
}
```
implicit self (rust style)

# Conditional defer

```tc
defer (condition){
    // code to be executed at the end of the scope if conditions are ok
}
```
Multiple conditions

defer (condition1 && condition2 ...){

}
defer (condition1 || condition2 ...){

}

## keywords for 1.2.1

1. if 
2. loop
3. break
4. defer
5. ret
6. strun
7. fn
8. use
9. pub 
10. pin
11. match
12. hot
13. else (new)
14. enum (new)

