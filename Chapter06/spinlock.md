**Purpose:**
- Mutual exclusion without kernel scheduling.
- Threads busy-wait instead of sleeping.

**Best for:**
- Low latency systems.
- When lock hold time is smaller than context switch cost (Very short critical sections.)

**Tradeoffs:**
+ Extremely low overhead under low contention.
+ No syscall/context switch.
- Burns CPU while waiting.
- Scales poorly under heavy contention.
- Can hurt fairness/starve threads.

**Core idea:**
```cpp
while(lock_taken)
    spin();
```

I used sleep after 8 tries, **backoff strategy** - after 8 failed spins, thread sleeps briefly to reduce contention and CPU usage.
It also reduces cache line bouncing between cores.