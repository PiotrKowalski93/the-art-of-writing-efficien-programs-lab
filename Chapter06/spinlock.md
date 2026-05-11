**Purpose:**
- Mutual exclusion without kernel scheduling.
- Threads busy-wait instead of sleeping.

**Best for:**
- Low latency systems.
- When lock hold time is smaller than context switch cost (Very short critical sections.)

**Tradeoffs:**
- :white_check_mark: Extremely low overhead under low contention.
- :white_check_mark: No syscall/context switch.
- :x: Burns CPU while waiting.
- :x: Scales poorly under heavy contention.
- :x: Can hurt fairness/starve threads.

**Core idea:**
```cpp
while(lock_taken)
    spin();
```

I used sleep after 8 tries, **backoff strategy** - after 8 failed spins, thread sleeps briefly to reduce contention and CPU usage.
It also reduces cache line bouncing between cores.