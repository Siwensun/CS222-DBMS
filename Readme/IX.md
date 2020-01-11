---
attachments: [B+tree.PNG]
tags: [Notebooks/CS222/IX]
title: IX
created: '2020-01-08T22:21:20.298Z'
modified: '2020-01-08T23:17:22.387Z'
---

# IX

![](@attachment/B+tree.PNG)

B+ tree based Index File System:
- Intermediate Node:
store key and ptr -> ptr points to the next intermediate node or leaf node.
Root node is like a top intermediate node

- Leaf Node:
store key and RID -> rid is used to retrieve the entire record.


**Page Format**:
- imPageDirectory: used in intermediate node
- leafPageDirectory: used in lead node, \<key, pointer\>
- stored in the beginnig of the page, \<key, data\>

**Key Format**:
- INT and REAL: 4 bytes
- VARCHAR: use 4 bytes for the length followed by the characters.






