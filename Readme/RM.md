---
tags: [Notebooks/CS222, Notebooks/CS222/RM]
title: RM
created: '2020-01-11T07:12:42.426Z'
modified: '2020-01-11T08:46:01.081Z'
---

# RM

The RelationManager class is responsible for managing the database tables. It handles the creation and deletion of tables. It also handles the basic operations performed on top of a table (e.g., insert and delete tuples).

## 1. Catalog
Create a catalog to hold all information about your database. This includes at least the following:

- Table information (e.g., table-name, table-id, etc.).
- For each table, the columns, and for each of these columns: the column name, type, length, and position.
- The name of the record-based file in which the data corresponding to each table is stored.

It is mandatory to store the catalog information by using the RBF layer functions. You should create the catalog's tables and populate them the first time your database is initialized (when the method createCatalog() is called). Once the catalog's tables and columns tables have been created, they should be persisted to disk. Please use the following name and type for the catalog tables and columns. You can add more attributes you want/need to. However, please do not change the given name of these two tables or their attribute names.

```
Tables (table-id:int, table-name:varchar(50), file-name:varchar(50))
Columns(table-id:int, column-name:varchar(50), column-type:int, column-length:int, column-position:int)
```

An example of the records that should be in these two tables after creating a table named "Employee" is:

```
Tables 
(1, "Tables", "Tables")
(2, "Columns", "Columns")
(3, "Employee", "Employee")

Columns
(1, "table-id", TypeInt, 4 , 1)
(1, "table-name", TypeVarChar, 50, 2)
(1, "file-name", TypeVarChar, 50, 3)
(2, "table-id", TypeInt, 4, 1)
(2, "column-name",  TypeVarChar, 50, 2)
(2, "column-type", TypeInt, 4, 3)
(2, "column-length", TypeInt, 4, 4)
(2, "column-position", TypeInt, 4, 5)
(3, "empname", TypeVarChar, 30, 1)
(3, "age", TypeInt, 4, 2)
(3, "height", TypeReal, 4, 3)
(3, "salary", TypeInt, 4, 4)
```

## 2.RelationManager class

> class RelationManager {}

First to remember is that catalog files are also tables, therefore initially there are two tables, "Tables" and "Columns". So when create all delete catalogs, it is just like the same operation on other tables, difference is that, we could hard code the names and attributes of catalog files.
```
Tables (table-id:int, table-name:varchar(50), file-name:varchar(50))
Columns(table-id:int, column-name:varchar(50), column-type:int, column-length:int, column-position:int)
Indexes(table-name:varchar(50), column-name:varchar(50), index-name:varchar(50))
```

Tricky, store current num of tables in the hidden file, after three pageCounters. This number could be used as the table id for new table after increment.











