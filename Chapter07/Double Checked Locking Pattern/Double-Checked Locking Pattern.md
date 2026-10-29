# Double-Checked Locking Pattern (DCLP)

## What is it for?

DCLP is used for accessing a shared object while avoiding a mutex on every subsequent access.

Example:

```text
1. Check without locking
2. If the object doesn't exist → acquire the lock
3. Check again
4. If it still doesn't exist → initialize it
```

This keeps the mutex off the common read path and only uses it during initialization.

## When to use it?

DCLP can be useful when:

* initialization is rare
* algo is reading heavy
* the object is accessed very frequently
* locking on the hot path is a measurable performance bottleneck
* you need explicit control over atomic operations and memory ordering

A correct modern C++ implementation typically requires `std::atomic` with appropriate **acquire/release memory ordering**.

## Alternatives

DCLP should not be the default choice. Prefer simpler mechanisms when possible.

### `static` local variable

```cpp
Config& get_config()
{
    static Config config;
    return config;
}
```

The simplest solution for lazy initialization. Initialization of a local `static` is thread-safe in modern C++.

### `std::call_once`

```cpp
std::call_once(flag, [] {
    // initialization
});
```

Useful when initialization is more complex or doesn't fit naturally into a local static.
A regular mutex is often the best choice when simplicity and correctness matter more than minimizing the cost of the hot path.
