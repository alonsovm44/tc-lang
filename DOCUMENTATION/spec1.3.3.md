# v1.3.3
Runtime patch

- [ ] Make async runtime production grade


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

# Keywords for 1.3.3


- **18 keywords** — 
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
`my` (new)