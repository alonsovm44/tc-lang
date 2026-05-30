# v 1.2.1

# enums
[APPROVED]
Simple enums since using match with magic numbers causes bugs

enum Color {
    RED
    GREEN
    BLUE
    BLACK
    WHITE
}

# if else 
using _f and _ is cumbersome

[solution]

Just implement `else` and nest if-else like in Go

# inlining

Inline C
```c
"C"{
    // C code
    int func(int x){
        return x*x;
    }
}
```

## [possible]
Inline assembly

Adding inline assembly
```s
"asm"{
    mov eax, 12
}

```
We could considere this later

# Pointer to strun acess
[DONE]
[pending]
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
}

Person person
person.name = "John"
person.sayHi() 


```

# conditional defer

defer (condition){
    // code to be executed at the end of the scope if conditions are ok
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

