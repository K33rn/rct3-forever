# Equivalence tests, one file per topic

Put the test for a new rewrite here, as `<topic>.inc` (the same topic name as its `forever/src/rewrite/<topic>.cpp`).
Don't add tests to `tools/equiv/equiv.cpp`: everyone used to append to the same few lines there, so two people
working at the same time always conflicted. Separate files never conflict.

`build.py test` finds every `*.inc` here by itself (it lists them in `out/equiv_tests.inc`). `equiv.cpp` reads each
file twice, so a file has two parts:

```cpp
// What the test covers.
#if EQUIV_PART == 1
// Stand-ins and helpers, at file scope (static functions, tables, globals).
// Everything equiv.cpp defines before tests() can be used: arena<T>(), fresh(), dev_note(), record(),
// stubs_for_now(), the shared stand-ins (desk_*, cheat_*, msg_*, ...).
#else
// The tests, inside tests(): build a Test and hand it over with quick(x) (or t.push_back(x) when the test must
// compare the game's .data too).
#endif
```

Give stand-ins and globals a prefix of your own (`ib_` for ident bars, say), so two files never define the same
name. [ident_bars.inc](ident_bars.inc) is a complete example.
