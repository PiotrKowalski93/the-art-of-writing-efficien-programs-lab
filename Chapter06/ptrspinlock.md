**Purpose:**
- Atomic ownership transfer of a pointer/resource.
- Synchronization + resource handoff in one operation.

**Best for:**
- Passing exclusive ownership between threads.
- Ultra low latency systems.
- Single shared resource/token patterns.

**Tradeoffs:**
+ Combines synchronization and pointer transfer.
+ Avoids separate mutex + lookup/access step.
+ Very cheap fast-path.
- Hard to understand/maintain.
- Easy to introduce races/UB.

**Core idea:**
exchange(pointer, nullptr)

nullptr  -> resource taken

pointer  -> resource available
