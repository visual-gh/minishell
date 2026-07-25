# src/env/: Environment Layer

The environment is what gives your shell access to variables like `PATH`, `HOME`, or `USER`.
When minishell starts, it receives those variables from the OS as a `char **`, a plain array of strings.
This layer owns a private heap copy of that array and exposes a small set of functions to read and mutate it.

It's the shell's long-lived state: created once at startup, read by the
[expander](../expander/README.md) (`$USER`) and the [executor](../executor/README.md)
(`$PATH` lookup), and written by the [`export`, `unset`, and `cd` builtins](../builtins/README.md).

---

## The data structure

The environment is a `NULL`-terminated array of `"KEY=VALUE"` strings:

```
shell->envp
│
▼
char *[0]  →  "PATH=/usr/bin:/bin\0"
char *[1]  →  "HOME=/home/visual\0"
char *[2]  →  "USER=visual\0"
char *[3]  →  NULL
```

Every string is heap-allocated and owned by the array.
The array itself is also heap-allocated.
That means two levels of `free`: each string, then the array pointer.

---

## Why `char ***envp` for set and unset?

`env_get` only reads, so `char **envp` is enough.

`env_set` may need to **grow** the array (new variable means realloc and replace pointer).
`env_unset` shifts entries in place to close the gap.

Both need to be able to **replace `shell->envp` itself**, so they take a pointer to it: `char ***envp`.
You call them with `&shell->envp`.

---

## Functions

### `env_index`: the one place that matches a key

Every operation below needs the same thing: *"which slot holds `KEY`?"* Rather
than write that scan three times, one helper owns it:

```c
int env_index(char **envp, const char *key);   // returns the index, or -1
```

It matches `"KEY=..."` by comparing the key and checking the character right after
it is `=` (so `PATH` doesn't match `PATHEXT`):

```c
if (ft_strncmp(envp[i], key, klen) == 0 && envp[i][klen] == '=')
    return (i);
```

`env_get`, `env_set`, and `env_unset` all call it. One place to be correct about
matching means they can never disagree.

---

### `env_init`: take ownership of the system environment

```c
char **env_init(char **envp);
```

Deep-copies the `envp` received by `main`. Every string is `strdup`'d into a fresh heap array.
After this call, the shell owns its environment independently and changes don't affect the parent process.

Returns the new array, or `NULL` on allocation failure.

```c
shell->envp = env_init(envp);
if (!shell->envp)
    // handle error
```

---

### `env_get`: read a variable

```c
char *env_get(char **envp, const char *key);
```

Scans the array for `"KEY=..."` and returns a pointer directly into that string, past the `=`.
No allocation: the returned pointer points inside the existing entry.

```c
char *path = env_get(shell->envp, "PATH");
// path == "/usr/bin:/bin"  (pointer into the live string)
```

> **Do not free the returned pointer.**
> Do not call `env_set` or `env_unset` on the same key while holding it, as the string it points into may be freed.

Returns `NULL` if the key doesn't exist.

---

### `env_set`: create or update a variable

```c
int env_set(char ***envp, const char *key, const char *val);
```

If `key` already exists: frees the old `"KEY=VALUE"` string and replaces it with a fresh one.
If `key` doesn't exist: grows the array by one slot and appends the new entry.

```c
env_set(&shell->envp, "PATH", "/usr/local/bin:/usr/bin");
env_set(&shell->envp, "MY_VAR", "hello");
```

Returns `0` on success, `-1` on allocation failure.
On failure, the environment is left **unchanged** and the failed allocation is cleaned up internally.

---

### `env_unset`: remove a variable

```c
void env_unset(char ***envp, const char *key);
```

Finds `"KEY=..."`, frees it, then shifts all subsequent pointers left to close the gap.
The last slot is set to `NULL`. No reallocation: the array pointer stays the same.

```c
env_unset(&shell->envp, "MY_VAR");
```

No-op if the key doesn't exist.

---

## Lifecycle

```
main() receives char **envp
        │
        ▼
  env_init()  →  shell->envp  (heap copy, we own it)
        │
        ├─ env_get()    read a value
        ├─ env_set()    write / create a value
        └─ env_unset()  remove a value
        │
        ▼
  shell_free() → free_str_array(shell->envp)
                 frees every string, then the array
```

---

## Memory rules

| What | Who owns it | When to free |
|---|---|---|
| `shell->envp` (the array) | you | `shell_free` via `free_str_array` |
| Each `"KEY=VALUE"` string | you | replaced by `env_set`, removed by `env_unset`, or freed at shutdown |
| Pointer returned by `env_get` | **not you** | never: it is a view into a live entry |
