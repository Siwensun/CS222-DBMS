---
tags: [Notebooks/CS222, Notebooks/CS222/QE]
title: QE
created: '2020-01-11T18:48:46.472Z'
modified: '2020-01-11T19:01:54.143Z'
---

# QE

Query Engine

## 1. Iterator Interface

All of the operators that you will implement in this part inherit from the following Iterator interface.
```
class Iterator {
    // All the relational operators and access methods are iterators
    // This class is the super class of all the following operator classes
public:
    virtual RC getNextTuple(void *data) = 0;

    // For each attribute in vector<Attribute>, name it rel.attr
    virtual void getAttributes(std::vector<Attribute> &attrs) const = 0;

    virtual ~Iterator() = default;
};
```

## 2. TableScan and IndexScan interface

- TableScan is a wrapper inheriting Iterator over RM_ScanIterator
- IndexScan is a wrapper inheriting Iterator over IX_ScanIterator

## 2. Operators

### 2.1 Filter Interface
```
class Filter : public Iterator {
    // Filter operator
public:
    Filter(Iterator *input,                       // Iterator of input R
           const Condition &condition     // Selection condition
    );

    ~Filter() override = default;

    RC getNextTuple(void *data) override;

    // For attribute in std::vector<Attribute>, name it as rel.attr
    void getAttributes(std::vector<Attribute> &attrs) const override;
};
```

### 2.2 Project Interface
```
class Project : public Iterator {
    // Projection operator
public:
    Project(Iterator *input,                                                  // Iterator of input R
            const std::vector<std::string> &attrNames) {};   // std::vector containing attribute names
    ~Project() override = default;

    RC getNextTuple(void *data) override;

    // For attribute in std::vector<Attribute>, name it as rel.attr
    void getAttributes(std::vector<Attribute> &attrs) const override;
};
```

### 2.3 Block Nested-Loop Join Interface
```
class BNLJoin : public Iterator {
    // Block nested-loop join operator
public:
    BNLJoin(Iterator *leftIn,            // Iterator of input R
            TableScan *rightIn,           // TableScan Iterator of input S
            const Condition &condition,   // Join condition
            const unsigned numPages       // # of pages that can be loaded into memory,
            //   i.e., memory block size (decided by the optimizer)
    ) {};

    ~BNLJoin() override = default;

    RC getNextTuple(void *data) override;

    // For attribute in std::vector<Attribute>, name it as rel.attr
    void getAttributes(std::vector<Attribute> &attrs) const override;
};
```

### 2.4 Index Nested-Loop Join Interface
```
class INLJoin : public Iterator {
    // Index nested-loop join operator
public:
    INLJoin(Iterator *leftIn,           // Iterator of input R
            IndexScan *rightIn,          // IndexScan Iterator of input S
            const Condition &condition   // Join condition
    ) {};

    ~INLJoin() override = default;

    RC getNextTuple(void *data) override;

    // For attribute in std::vector<Attribute>, name it as rel.attr
    void getAttributes(std::vector<Attribute> &attrs) const override;
};
```


