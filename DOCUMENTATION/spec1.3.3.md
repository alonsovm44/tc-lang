# v1.3.3
Runtime patch

[ ] - Fix async runtime

## Pointers to stacks and queues

It partially works now

```
fn void main: {
    queue<i32> q
    ->queue<i32> qptr = &q


}

```

## Library for hash maps

[ ] - Create a library for hash maps type

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
