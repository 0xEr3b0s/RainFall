# Rainfall

> A binary exploitation project of the 42 Cybersecurity curriculum.

## Overview

**Rainfall** is an exercise in the art of software exploitation. It presents a
succession of vulnerable programs — compiled C and C++ binaries — each of which
must be understood, dissected, and ultimately subverted. The task is not merely
academic: every level is a locked door, and the key to each lies in the level
that precedes it.

The project furnishes a dedicated virtual machine upon which a series of users
reside, from `level0` through `level9`, accompanied by several `bonus` stages.
Each user owns a **SUID binary** — a program that executes with the privileges of
its owner rather than those of the caller. By discovering and exploiting a flaw
within a given binary, one obtains a shell bearing the identity of the next user,
and may thereby read the password that grants passage to the following level.

Progress is thus a chain: comprehension yields access, access yields the next
secret, and the next secret yields the next challenge — each rung more demanding
than the last.

## Objectives

The project is designed to cultivate a practitioner's intuition for the manner in
which programs fail, and for the discipline required to turn that failure to one's
advantage. In the course of it, one learns to:

- Read and interpret compiled binaries at the level of assembly, absent any
  source.
- Recognise the signatures of classical vulnerability families in native code.
- Reason about memory layout — the stack, the heap, and the segments between.
- Craft precise inputs that redirect a program's execution toward an end of one's
  own choosing.
- Employ the standard instruments of static and dynamic analysis with fluency.

## Concepts Encountered

Across its levels, Rainfall traverses a broad territory of exploitation
technique, including but not confined to:

- **Buffer overflows** — the overrun of a fixed allocation and the corruption of
  adjacent memory.
- **Format string vulnerabilities** — the abuse of an unguarded conversion
  specifier to read from and write to arbitrary addresses.
- **Return address control** — the seizure of a function's return and the
  redirection of control flow.
- **Return-to-libc and code reuse** — the assembly of an exploit from the
  program's own existing machinery.
- **GOT and PLT manipulation** — the diversion of a program's indirect calls
  through its linkage tables.
- **C++ object internals** — the exploitation of virtual method tables and the
  particulars of object memory in the later, more intricate stages.
- **Environment and argument manipulation** — the placement of chosen data within
  a process's reach.

## Methodology

The prescribed method is one of patience rather than force. For each binary:

1. **Reconnaissance** — establish the file's nature, its protections, and its
   permissions.
2. **Static analysis** — read the disassembly, recover the program's logic, and
   locate the point of weakness.
3. **Dynamic analysis** — observe the program under execution, confirm the
   hypothesis, and measure the exact contours of the flaw.
4. **Exploitation** — construct the input that transforms understanding into
   access.
5. **Ascent** — recover the next password and proceed.

## Tooling

The work is conducted upon a **32-bit x86** system and relies upon the customary
apparatus of the discipline:

- `gdb` — the debugger, for dynamic inspection of execution and memory.
- `objdump` — for static disassembly and the reading of section headers.
- `ltrace` and `strace` — for the tracing of library and system calls.
- `checksec` — for the enumeration of a binary's defensive measures.
- **Python** — for the scripting of payloads and the marshalling of precise input.

## Repository Contents

This repository documents the author's traversal of the project. Each level folder
holds the reconstructed source, a written analysis of the vulnerability, and the
reasoning by which it was exploited.

## A Note on Ethics

The techniques rehearsed here are the instruments of the offensive-security
practitioner, studied in a controlled and sanctioned environment for the express
purpose of learning to defend. They are to be applied only upon systems for which
one holds explicit authorisation. Knowledge of how a lock may be picked is the
locksmith's trade; it is not a licence to enter another's house.

---

*Part of the 42 Cybersecurity curriculum — the binary exploitation chain.*
