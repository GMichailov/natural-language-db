# Phase 3: Batched Write Mode and LLM Frontend

## Goal

Add MiniDB's read/write phase model and an optional natural-language frontend. This phase turns the database into a more distinctive system: one that supports explicit read phases, explicit batched write phases, and safe translation from natural language into the MiniDB DSL.

The main goal is to support this flow:

```text
Natural language input
  -> small local model
  -> MiniDB DSL
  -> parser
  -> validator
  -> planner
  -> executor
```

The LLM should never directly control the database engine. It should only produce DSL text, which MiniDB then parses, validates, plans, and executes.

## Required Features

### Explicit Read and Write Modes

MiniDB should support a global engine mode:

```cpp
enum class EngineMode {
    READ,
    WRITE
};
```

In `READ` mode:

```text
SELECT is allowed
INSERT is rejected
CREATE TABLE is rejected or restricted
CREATE INDEX is rejected or restricted
```

In `WRITE` mode:

```text
INSERT is allowed
CREATE TABLE may be allowed
CREATE INDEX may be allowed
SELECT is rejected or explicitly unavailable
```

Suggested commands:

```sql
BEGIN READ;
SELECT * FROM users;
END;

BEGIN WRITE;
INSERT INTO users VALUES (1, 24);
INSERT INTO users VALUES (2, 31);
COMMIT;
```

### Batched Writes

During write mode, MiniDB should collect and apply writes in a controlled batch.

Simple implementation:

```text
BEGIN WRITE
  execute inserts immediately, but mark pages dirty
COMMIT
  flush dirty pages
  update catalog metadata
  return to READ mode
```

More advanced implementation:

```text
BEGIN WRITE
  collect pending writes
COMMIT
  sort or batch writes
  apply table inserts
  apply index inserts
  flush dirty pages
  atomically publish metadata
  return to READ mode
```

The simple implementation is enough for the first version.

### Mode Enforcement

Every statement should be checked against the current engine mode.

Example:

```text
db> begin read;
OK

db> insert into users values (1, 24);
ERROR: INSERT is not allowed in READ mode
```

And:

```text
db> begin write;
OK

db> select * from users;
ERROR: SELECT is not allowed in WRITE mode
```

This gives the database a clear and intentional concurrency model.

### Query Validation

Before execution, validate that a query is legal.

Validation should check:

- The table exists
- Referenced columns exist
- Inserted values match the table schema
- The query is allowed in the current engine mode
- LLM-generated queries use only supported syntax

This is especially important once natural-language translation is added.

### Natural-Language Frontend

Add a separate model runner module.

The model runner should expose a clean interface:

```cpp
class ModelRunner {
public:
    std::string generate_query(const std::string& natural_language,
                               const Catalog& catalog);
};
```

Input:

```text
show me all users older than 21
```

Output:

```sql
SELECT * FROM users WHERE age > 21;
```

The model should be treated as an untrusted query generator.

The database must still:

1. Parse the generated DSL
2. Validate it
3. Reject unsupported or unsafe queries
4. Execute only valid DSL

### REPL Support for NL Mode

Add a way to distinguish DSL from natural language.

Example:

```text
db> select * from users where age > 21;
```

Direct DSL execution.

```text
db> :nl show me all users older than 21
```

Natural-language translation path.

Output:

```text
Generated query:
SELECT * FROM users WHERE age > 21;

id | age
1  | 24
2  | 31
```

### Model Runner Stub First

Before loading a real model, implement a fake model runner.

Example:

```cpp
std::string ModelRunner::generate_query(const std::string& input,
                                        const Catalog& catalog) {
    if (input == "show all users") {
        return "SELECT * FROM users;";
    }
    return "";
}
```

This allows the rest of the NL pipeline to be tested before integrating LibTorch, llama.cpp, ONNX Runtime, or another local inference backend.

### Real Model Integration

Once the stub works, replace it with a small CPU-capable model.

Possible runners:

```text
LibTorch
llama.cpp
ONNX Runtime
custom lightweight inference wrapper
```

The model does not need highly optimized kernels. Its only job is to translate user requests into a tiny DSL.

A good prompt format:

```text
You translate user requests into MiniDB query language.

Schema:
users(id INT, age INT)

Supported syntax:
SELECT columns FROM table WHERE column op value;
INSERT INTO table VALUES (...);

User request:
show me all users older than 21

Return only MiniDB DSL.
```

Expected output:

```sql
SELECT * FROM users WHERE age > 21;
```

## Milestones

### Milestone 1: Engine Modes

Done when:

- `BEGIN READ` works
- `BEGIN WRITE` works
- `COMMIT` works for write mode
- Invalid statements are rejected based on mode

### Milestone 2: Batched Write Commit

Done when:

- Multiple inserts can be executed in one write phase
- Dirty table pages are flushed on commit
- Index updates are applied correctly
- The database returns to read mode after commit

### Milestone 3: Query Validation

Done when:

- Invalid table names are rejected
- Invalid column names are rejected
- Type mismatches are rejected
- Unsupported syntax is rejected
- Mode violations are rejected

### Milestone 4: Stub NL-to-DSL Runner

Done when:

- `:nl ...` routes input through a model runner interface
- The stub model can return hardcoded DSL
- Generated DSL is parsed and validated
- Invalid generated DSL is rejected safely

### Milestone 5: Real Local Model Runner

Done when:

- A small local model can be loaded
- The model receives schema context
- The model returns MiniDB DSL
- The output is validated before execution
- Simple natural-language queries work end-to-end

## Example Demo

```text
db> begin write;
OK

db> insert into users values (1, 24);
OK

db> insert into users values (2, 31);
OK

db> commit;
OK. Returned to READ mode.

db> :nl show me users older than 25
Generated query:
SELECT * FROM users WHERE age > 25;

id | age
2  | 31
```

## Done Criteria

Phase 3 is complete when MiniDB can:

- Enforce explicit read and write modes
- Batch writes within a write phase
- Commit writes and return to read mode
- Validate queries before execution
- Route natural-language input through a model runner
- Convert simple natural-language requests into MiniDB DSL
- Safely reject invalid model output
- Execute valid model-generated queries

## Explicit Non-Goals

Do not implement these yet:

- Full transaction rollback
- WAL-based crash recovery
- MVCC
- Concurrent reads during writes
- Concurrent writes during reads
- Fine-grained locks
- General-purpose SQL generation
- Unrestricted model-generated commands
- Autonomous model execution without validation
