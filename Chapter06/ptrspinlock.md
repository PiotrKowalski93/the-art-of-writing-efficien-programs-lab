**Purpose:**
- Atomic ownership transfer of a pointer/resource.
- Synchronization + resource handoff in one operation.

**Best for:**
- Passing exclusive ownership between threads.
- Ultra low latency systems.
- Single shared resource/token patterns.

**Tradeoffs:**
- :white_check_mark: Combines synchronization and pointer transfer.
- :white_check_mark: Avoids separate mutex + lookup/access step.
- :white_check_mark: Very cheap fast-path.
- :x: Hard to understand/maintain.
- :x: Easy to introduce races/UB.

**Core idea:**
exchange(pointer, nullptr)

nullptr  -> resource taken

pointer  -> resource available
