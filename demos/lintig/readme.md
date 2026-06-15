# Lintig
Lintig is a static analysis tool that checks your code for errors and style issues. And memory leaks within your Tig program. 

## Usage
`lintig <file.tc> --[fags]`
## Flags
- `-h, --help`: Show help message
- `-v, --version`: Show version number
- `-p, --pedantic`: Enable pedantic mode
- `-vr, --verbose`: Enable verbose output

## MVP
Version 1.0.0
- Track memory leaks 
    - Allocations without defer stmts or manual free in the same scope

- Unused variables
- Unused functions

## Future

- Use-after-free detection
- Double free detection
- Escape analysis
- Missing defer suggestions
- Function annotation support