# SimpleDBMS

SimpleDBMS is a lightweight, educational relational database management system built in C++17. It features a custom SQL-like query language, dynamic database file switching, in-memory table structures, and data durability using Write-Ahead Logging (WAL). 

This project is built from scratch with a modular storage engine, schema catalog, and query execution engine without using external dependencies like SQLite.

## Core Features
1. **Interactive REPL Engine:** A persistent command-line interface for sending real-time queries.
2. **Schema Engine:** Native support for arbitrary schemas utilizing `INTEGER` and `VARCHAR` (Strings).
3. **Data Persistence (WAL):** Absolute guaranteed data durability. Every table creation and row insertion streams directly to a `<database>.log` Write-Ahead Log. On boot, the DBMS replays these logs in milliseconds to perfectly reconstruct the state of all data.
4. **Dynamic Database Connectivity:** Boot the system offline and manually jump between different `.db` environments at runtime (like `connect users`, `connect inventory`, etc).
5. **Custom SQL-like Syntax:** Lightweight, simplified syntax replacing standard database overhead commands.

## Getting Started

### Prerequisites
- C++17 Compatible Compiler (e.g. GCC / g++) 

### Compilation
Build the project natively via your terminal:
```bash
g++ -std=c++17 -Iinclude src/main.cpp src/storage/disk_manager.cpp src/storage/buffer_pool_manager.cpp -o dbms.exe
```

### Running the Engine
Simply boot the generated executable. It will launch into an unconnected state where no data can be queried until you dynamically mount a database.
```bash
./dbms.exe
```
*(Optionally, you can supply a default database at boot: `./dbms.exe my_database`)*

---

## Query Syntax Guide

The syntax of SimpleDBMS is designed to be slightly more verbose and colloquial than standard SQL. Ensure you connect to a database before attempting any queries.

### `connect <database>`
Dynamically connect to a database environment. It automatically generates the storage files if they don't exist and immediately replays its Write-Ahead Log.
```sql
dbms> connect data.db
[INFO] Connected to database: data.db
```

### `make table <table> (<col1> <type>, ...)`
Equivalent to SQL's `CREATE TABLE`. Architect a new schema containing columns typed as `INT` or `VARCHAR`/`STRING`.
```sql
dbms> make table users (id int, name varchar, city string)
```

### `insert into <table> values (<val1>, ...)`
Insert a row of values mapped accurately to the data types defined during `make table`.
```sql
dbms> insert into users values (1, 'Alice', 'New York')
```

### `show database` (or `show`)
Display all currently active tables loaded inside the memory of the connected database.
```sql
dbms> show database
[INFO] Tables in database 'data.db':
- users
```

### `show all from <table>`
Equivalent to SQL's `SELECT *`. Dumps all data rows using formatted grid styling.
```sql
dbms> show all from users
```

### `show all from <table> where <col> = <val>`
Equivalent to SQL's `SELECT * WHERE`. Applies conditional filtering logic against the requested string or integer target.
```sql
dbms> show all from users where id = 1
```

### `change <table> set <col> = <val>`
Equivalent to SQL's `UPDATE`. Modifies the column of an existing table across every single row. 
```sql
dbms> change users set city = 'Los Angeles'
```

### `remove from <table> where <col> = <val>` (or `delete from ...`)
Equivalent to SQL's `DELETE FROM`. Removes rows from the given table where the specific conditional matching is strictly met.
```sql
dbms> remove from users where name = 'Alice'
dbms> delete from users where id = 1
```

## Internal Architecture
* **`DiskManager`**: Interface abstracting file I/O operations and allocating/deallocating space using `Page` blocks dynamically. Built internally around basic C++ standards.
* **`BufferPoolManager`**: High-level in-memory page cache (acting on top of the Disk Manager) that pulls and pushes database shards, cycling them autonomously utilizing a Least-Recently-Used (LRU) algorithm.
* **`Catalog`**: Internal namespace mapped using `std::unordered_map` orchestrating logical constructs (`TableInfo`, `Schema`, `Column`).
* **`Tuple`**: Basic row architecture storing abstract arrays of `Value` objects dynamically.
* **`WAL Engine`**: Sequential instruction persistence mechanism running directly inside `Execute` hooks to stream changes to local `<db_file>.log` environments.
