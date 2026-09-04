# Level XX — Walkthrough

---

## Overview

| | |
|---|---|
| **Binary** | `levelXX` |
| **User** | `levelXX` |
| **Goal user** | `levelXX+1` |
| **Protections** | *(output of checksec)* |
| **Password (current level)** | *(password used to reach this user)* |
| **Password (obtained)** | *(password extracted for the next user)* |

---

## 1. Reconnaissance

> What manner of binary do we face? What does it expect of us, and by what means?

- File type, architecture, and linking (`file`, `ldd`)
- Enabled protections (canary, NX, PIE, RELRO)
- Behaviour under ordinary use — arguments, environment, stdin

---

## 2. Static Analysis

> A study of the binary's anatomy prior to any disturbance of it.

- Notable functions and their purpose
- Suspicious calls (`gets`, `strcpy`, `printf` without format, `system`, etc.)
- Strings of interest
- Structure of the stack frame, so far as it may be inferred

---

## 3. Dynamic Analysis

> Observation of the binary in the course of its execution.

- Breakpoints set and their justification
- Behaviour observed at the moment of the suspected fault
- Offset to the return address / value of interest, and how it was established

---

## 4. Vulnerability

> A plain statement of the flaw, once it has been identified.

- Nature of the vulnerability
- Why the protections in place do not prevent its exploitation

---

## 5. Exploitation

> The reasoning behind the chosen approach, set down before the means themselves.

- Strategy adopted (ret2text, ret2libc, format string write, etc.)
- Construction of the payload — reasoning, not merely the result
- Any obstacle encountered, and how it was overcome

---

## 6. Result

- Password obtained for the next user
- Final command or sequence that yielded it

---

## Notes & Lessons

> What this level taught, beyond the mere solving of it.
