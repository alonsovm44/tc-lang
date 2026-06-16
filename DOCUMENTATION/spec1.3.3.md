# v1.3.3
Runtime patch

- [ ] Make async runtime production grade


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
