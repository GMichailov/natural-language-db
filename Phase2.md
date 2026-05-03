# Phase 2: Disk Storage and Indexing

## Goal

Turn MiniDB from an in-memory toy database into a real disk-backed database engine. This phase introduces fixed-size pages, a disk manager, a buffer pool, heap-file table storage, record IDs, and a B+Tree index.

The main goal is to support this flow:

```text
Query plan
  -> table scan or index scan
  -> buffer pool
  -> disk pages
  -> rows
```

By the end of this phase, MiniDB should be able to show a measurable difference between a full table scan and an indexed lookup.

## Why This Phase Matters

This is where the project becomes a real database engine.

A database is not just a parser over vectors. It needs to manage data on disk, load pages into memory, track dirty pages, assign record locations, and use indexes to avoid scanning entire tables.

## Required Features

### Fixed-Size Pages

Represent database files as fixed-size pages.

Recommended page size:

```cpp
constexpr size_t PAGE_SIZE = 4096;
```

A database file should be treated as:

```text
page 0
page 1
page 2
page 3
...
```

Each page is read and written as a unit.

### Disk Manager

Implement a component responsible for raw file I/O.

Suggested interface:

```cpp
class DiskManager {
public:
    Page read_page(PageId page_id);
    void write_page(PageId page_id, const Page& page);
    PageId allocate_page();
};
```

The disk manager should not understand SQL, rows, tables, or indexes. It only reads and writes pages.

### Page Layout

For the first disk-backed version, use fixed-size rows.

Example table:

```sql
CREATE TABLE users (id INT, age INT);
```

Each row is 8 bytes:

```text
id:  4 bytes
age: 4 bytes
```

A table page can contain:

```text
+----------------------+
| Page header          |
| - page id            |
| - number of records  |
| - next page id       |
+----------------------+
| row 0                |
| row 1                |
| row 2                |
| ...                  |
+----------------------+
```

Strings and variable-length rows should be deferred until later.

### Record IDs

Introduce a stable row location type.

```cpp
struct RID {
    PageId page_id;
    uint16_t slot_id;
};
```

The heap table insert path should return an RID:

```cpp
RID rid = table.insert(row);
```

The RID allows indexes to point to table records.

### Heap Table

Implement disk-backed table storage.

The heap table should support:

```cpp
RID insert(const Row& row);
Row read(RID rid);
std::vector<Row> scan();
```

For this phase, rows can be append-only. Do not implement delete or update yet.

### Buffer Pool

Add a buffer pool between the executor and disk manager.

The buffer pool caches disk pages in memory.

Suggested interface:

```cpp
class BufferPool {
public:
    Page& fetch_page(PageId page_id);
    void mark_dirty(PageId page_id);
    void flush_page(PageId page_id);
    void flush_all();
};
```

The executor and table storage should not directly call the disk manager once the buffer pool exists.

Instead of:

```cpp
disk.read_page(page_id);
```

use:

```cpp
Page& page = buffer_pool.fetch_page(page_id);
```

Start with a very simple eviction policy. LRU or Clock can be added later.

### B+Tree Index

Implement a B+Tree index for integer keys.

The index maps:

```text
key -> RID
```

Example:

```text
42 -> { page_id: 7, slot_id: 3 }
```

Required operations:

```cpp
void insert(int key, RID rid);
std::optional<RID> find(int key);
std::vector<RID> range_scan(int min_key, int max_key);
```

The first version can support:

- Integer keys only
- Unique keys only
- Insert and lookup
- Optional range scan
- No delete

### B+Tree Page Types

Use separate node types:

```text
Internal page:
  keys
  child page ids

Leaf page:
  keys
  RIDs
  next leaf page id
```

Leaf pages should be linked to support range scans.

```text
Leaf A -> Leaf B -> Leaf C
```

### CREATE INDEX

Add query support for:

```sql
CREATE INDEX ON users(id);
```

When this command runs, MiniDB should:

1. Scan the table
2. Extract the indexed column from each row
3. Insert `key -> RID` into the B+Tree
4. Register the index in the catalog

### Index-Aware Planner

Teach the planner to choose between:

```text
TableScan + Filter
```

and:

```text
IndexScan
```

Example:

```sql
SELECT * FROM users WHERE id = 5;
```

If an index exists on `users.id`, use:

```text
IndexScan(users, index=id, key=5)
```

Otherwise use:

```text
Filter(id = 5)
  TableScan(users)
```

## Milestones

### Milestone 1: Disk Manager and Pages

Done when:

- Pages can be allocated
- Pages can be written to disk
- Pages can be read back correctly
- Data persists across program restarts

### Milestone 2: Disk-Backed Heap Table

Done when:

- Rows are inserted into table pages
- Inserts return valid RIDs
- Rows can be read by RID
- Full table scans work from disk

### Milestone 3: Buffer Pool

Done when:

- Page reads go through the buffer pool
- Dirty pages are tracked
- Dirty pages are flushed correctly
- The engine still works with a small buffer pool

### Milestone 4: B+Tree Insert and Lookup

Done when:

- Integer keys can be inserted
- Keys can be found after insertion
- Leaf node splitting works
- Internal node splitting works
- Root splitting works

### Milestone 5: Planner Uses Indexes

Done when:

- `CREATE INDEX ON table(column)` works
- The catalog tracks indexes
- Equality queries use index scans when possible
- Queries still fall back to table scans when no index exists

## Example Demo

```text
db> create table users (id int, age int);
OK

db> insert into users values (1, 24);
OK

db> insert into users values (2, 31);
OK

db> create index on users(id);
OK

db> select * from users where id = 2;
id | age
2  | 31
```

Debug output could show:

```text
Plan: IndexScan(users.id = 2)
```

## Benchmark Goal

Create a benchmark that compares:

```sql
SELECT * FROM users WHERE id = 834221;
```

With no index:

```text
Plan: TableScan + Filter
```

With an index:

```text
Plan: IndexScan
```

This benchmark should demonstrate why B+Trees matter.

## Done Criteria

Phase 2 is complete when MiniDB can:

- Persist table data to disk
- Read table data after restarting the program
- Insert rows into heap pages
- Identify rows using RIDs
- Cache pages through a buffer pool
- Build an integer B+Tree index
- Use the B+Tree for indexed lookups
- Choose between table scans and index scans in the planner

## Explicit Non-Goals

Do not implement these yet:

- Deletes
- Updates
- B+Tree deletion or rebalancing
- Variable-length strings
- Crash recovery
- WAL
- MVCC
- Concurrent readers and writers
- LLM integration
