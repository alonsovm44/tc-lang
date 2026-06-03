# fixes pending

 - New hot reloading not working.

## Problems
@ overloading
1. The @ Operator Has 4+ Meanings

| Version | Usage | Meaning |
|---------|-------|---------|
| 1.0.0 | @var | Address-of |
| 1.1.0 | @use "file.tc" | Compile-time file inclusion |
| 1.1.0 | @strun | Packed struct |
| 1.3.0 | @x | Give ownership to async function |
| 1.4.0 | @x | Send to channel |

### Solution

&var -> adress of
inline use "file.tc" -> compile-time file inclusion
This allows to reuse the word `inline` in 
`inline "C"{}` for C interop

!strun -> packed struct
@x -> give ownership to async function
@x -> send to channel