<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: cac06a204874f082
timestamp: 2026-06-07T22:15:35Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# checker.h

## Declaration of Scope and VarInfo Structures
```markdown
typedef struct {
    char *name;
    Type *type;
    bool is_owned;     // Variable ownership status
    bool is_dead;       // Variable is dead (ownership transferred)
    bool is_pinned;     // Variable is pinned (immutable)
} VarInfo;

typedef struct {
    Scope *vars;
    int count;
    int cap;
} Scope;

typedef struct {
    Scope *scopes;
    int depth;
    int cap;
    DeclVec *program;
    Decl *current_fn;  // Current function being checked
} ScopeStack;
```

This section defines the `VarInfo` structure which holds information about variable definitions in a scope, such as its name, type, and ownership status. It also introduces the `Scope` and `ScopeStack` structures used for managing multiple scopes within a program.

## Function: push_scope

### Purpose
Pushes an entry onto the stack of function-level scopes (used to track nested functions and their variables).

### Usage & Behavior
- The function allocates memory for a new scope if necessary, extending its capacity as required.
- It then pushes this new scope into the `scopes` array. The depth is incremented accordingly.

## Function: pop_scope

### Purpose
Pops off a stack entry representing an entire function-level scope and frees any allocated resources associated with it (e.g., variable lists).

### Usage & Behavior
- Frees memory occupied by all variables in the popped-off scope if they were dynamically allocated.
- Decrement depth counter.

## Function: declare_var

### Purpose
Registers a new local variable within an existing function-level scope, allocating space for this information if necessary and adding it to the list of known variables (`vars`).

### Usage & Behavior
- Adds a `VarInfo` structure to the end of the `Scope.vars` array.
- If no entry exists yet in `scopes`, one is created by allocating memory.

## Function: get_var_info

### Purpose
Retrieves information about a variable from any given scope within the program's stack. Returns `NULL` if the variable cannot be found.

### Usage & Behavior
- Searches downwards through scopes until either the requested variable is found or all scopes have been checked.
- If the variable is not found, it returns `NULL`.

## Function: mark_var_dead

### Purpose
Marks a variable as dead (indicating its ownership has been transferred elsewhere), updating any relevant information about that variable.

### Usage & Behavior
- Checks if the current scope contains the requested variable.
- If so, marks the variable's state to indicate it is now dead.
- No direct effect on program execution; used for error reporting.

## Function: check_var_access

### Purpose
Checks if a variable access attempt within an accessible function-level scope has a valid context (i.e., isn't accessing a dead variable).

### Usage & Behavior
- Compares the provided `name` against known variables in the current scope.
- If a match is found and the variable is marked as dead, it prints an error message.

## Function: var_in_scope

### Purpose
Determines whether a given name exists within any of the function-level scopes. Used to verify if variables are correctly scoped.

### Usage & Behavior
- Iterates upwards through the scope stack (i.e., from outermost functions inward).
- For each entry, checks all known variable entries for matches.
- Returns `true` if found; otherwise, it continues searching.

## Function: get_var_type

### Purpose
Retrieves the type of a given variable within an accessible function-level scope.

### Usage & Behavior
- Searches downwards through scopes until either the requested variable is found or all scopes have been checked.
- If not found, `NULL` is returned. Otherwise, it returns the associated type.

## Function: fn_exists

### Purpose
Determines whether a given name corresponds to any defined function within the provided program vector (`DeclVec*`). Used for error reporting purposes.

### Usage & Behavior
- Iterates through each element in the vector.
- Compares the current declaration's name against the search term; if they match, it returns `true`.
- Returns `false` otherwise.

## Function: is_assignment

### Purpose
Checks whether a given operator indicates an assignment operation. Used to enforce certain types of variable manipulations (e.g., preventing writes to pinned variables).

### Usage & Behavior
- Compares the provided string (`op`) against common assignment operators (`="`, `+=`, `-=`) and returns `true` for matches.
- Returns `false` otherwise.

## Function: check_expr

### Purpose
Checks expressions within a function-level scope, ensuring their types and accesses are valid. Invoked recursively to handle nested structures (e.g., if/else statements).

### Usage & Behavior
- Depending on the type of expression (`EX_NAME`, `EX_CALL`, etc.), additional checks or transformations may be applied.
- For example, it ensures that variables accessed via name references exist within the current scope and have appropriate types.

## Function: check_stmts

### Purpose
Traverses through a vector of statements to perform their respective checks. Invoked recursively for loops and if/else constructs.

### Usage & Behavior
- Depending on the type of statement (`ST_VAR`, `ST_EXPR`, etc.), additional logic may be applied.
- For example, it handles variable declarations by checking expressions and updating scopes accordingly.

## Function: check_program

### Purpose
Performs a comprehensive analysis over a program's declaration vector. Ensures all named entities (variables/methods/fields) are appropriately scoped and do not conflict with each other or the user-defined interface of functions within those types.

### Usage & Behavior
- It first verifies that no method/field name conflicts exist within structs by comparing methods against fields.
- Then, for each declared function:
  - A new scope stack is created to track nested scopes.
  - Function parameters are added to this scope (if they are local).
  - The body of the function is analyzed using `check_stmts`.
  - Any dynamically allocated variables' storage space is freed when exiting the scope.

## Conclusion

This comprehensive set of functions works together to ensure that a program's code adheres to certain structural and logical rules, including proper scoping, type consistency, and no method/field name conflicts. It does so by recursively traversing through both expressions and statements within functions and verifying their integrity at every level.