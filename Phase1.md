# Phase 1: Core Query Engine

## Goal

Build the first usable version of MiniDB as an in-memory database with a custom query language. This phase proves that the frontend, parser, AST, planner, and executor work before adding disk storage or indexes.

The main goal is to support this flow:

```text
Terminal input
  -> lexer
  -> parser
  -> AST
  -> planner
  -> execution plan
  -> in-memory table execution
  -> printed result
```

## Why This Phase Comes First

The database engine should be correct and debuggable before adding disk pages, buffer management, B+Trees, or an LLM frontend. If the query language and execution model are unstable, every later layer becomes harder to test.

This phase deliberately keeps storage simple by using in-memory tables such as `std::vector<Row>`.

## Required Features

### Terminal REPL

Implement a basic terminal loop:

```text
db> create table users (id int, age int);
db> insert into users values (1, 24);
db> select * from users;
db> select * from users where id = 1;
```

The REPL should:

- Read a query string from stdin
- Send it through the parser
- Execute it
- Print results or errors
- Continue until the user exits

### Lexer

Convert source text into tokens.

Example input:

```sql
SELECT * FROM users WHERE id = 5;
```

Example token stream:

```text
SELECT
STAR
FROM
IDENTIFIER(users)
WHERE
IDENTIFIER(id)
EQUALS
INTEGER(5)
SEMICOLON
```

The lexer should recognize:

- Keywords: `CREATE`, `TABLE`, `INSERT`, `INTO`, `VALUES`, `SELECT`, `FROM`, `WHERE`, `INT`
- Identifiers
- Integer literals
- String literals if strings are included in this phase
- Symbols: `*`, `,`, `;`, `(`, `)`, `=`, `<`, `>`

### Parser

Convert tokens into an AST.

Supported statements:

```sql
CREATE TABLE users (id INT, age INT);
INSERT INTO users VALUES (1, 24);
SELECT * FROM users;
SELECT id, age FROM users WHERE id = 1;
```

Recommended AST node types:

```cpp
struct CreateTableStatement;
struct InsertStatement;
struct SelectStatement;
struct WhereClause;
```

### Catalog

Maintain table metadata.

The catalog should track:

- Table names
- Column names
- Column types
- Column order

Example:

```text
users:
  id: INT
  age: INT
```

### In-Memory Table Store

Represent tables in memory.

A simple version could use:

```cpp
struct Row {
    std::vector<Value> values;
};

struct Table {
    Schema schema;
    std::vector<Row> rows;
};
```

The first version can support only `INT` columns. Strings can be added later if desired.

### Planner

Convert AST statements into execution plans.

Initial plan nodes:

```text
CreateTablePlan
InsertPlan
TableScanPlan
FilterPlan
ProjectionPlan
```

Example:

```sql
SELECT id FROM users WHERE age > 21;
```

Planner output:

```text
Projection(id)
  Filter(age > 21)
    TableScan(users)
```

### Executor

Execute plans against the in-memory table store.

A simple iterator model is recommended:

```cpp
class PlanNode {
public:
    virtual std::optional<Row> next() = 0;
};
```

The executor repeatedly calls `next()` until the plan is exhausted.

## Milestones

### Milestone 1: REPL and Lexer

Done when:

- The REPL accepts user input
- The lexer prints or returns correct tokens
- Invalid characters produce useful errors

### Milestone 2: Parser and AST

Done when:

- `CREATE TABLE`, `INSERT`, and `SELECT` parse successfully
- Invalid syntax produces useful errors
- AST nodes can be printed for debugging

### Milestone 3: In-Memory Execution

Done when:

- Tables can be created
- Rows can be inserted
- `SELECT * FROM table` works
- `SELECT columns FROM table` works

### Milestone 4: Filtering

Done when:

- `WHERE col = value` works
- `WHERE col > value` works
- `WHERE col < value` works

## Example Demo

```text
db> create table users (id int, age int);
OK

db> insert into users values (1, 24);
OK

db> insert into users values (2, 31);
OK

db> select * from users;
id | age
1  | 24
2  | 31

db> select id from users where age > 25;
id
2
```

## Done Criteria

Phase 1 is complete when MiniDB can:

- Accept queries through a terminal REPL
- Parse a small SQL-like DSL
- Maintain table schemas in a catalog
- Store rows in memory
- Execute basic inserts and selects
- Apply simple filters
- Print useful query results
- Report useful syntax and runtime errors

## Explicit Non-Goals

Do not implement these yet:

- Disk storage
- Buffer pool
- B+Tree indexes
- Joins
- Delete/update
- Transactions
- LLM integration
- Query optimization beyond simple planning
