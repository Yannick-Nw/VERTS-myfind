# Project MYFIND - Grundlagen verteilter Systeme (VERTS)
Group project for third semester course "Grundlagen verteilter Systeme" (VERTS) at UAS Technikum Wien.

Authors: Matthias Kemmer, Yannick Nwankwo

### Task
Write a program in C/C++ that enables a user to parallelly find different files in a folder. Do not use the linux command 'find' as a child process (or somehow else).

### Build with Terminal
Build command: `g++ -g -Wall -o myfind main.cpp`

### Build with Makefile
Build all command: `make all`

Build myfind command: `make myfind`

Clean command: `make clean`

### Usage:
Command: `./myfind [-R] [-i] searchpath filename1 [filename2] … [filenameN]`

#### Arguments:
**-R** (optional) switch myfind in recursive mode and find all matching files in and below the
searchpath folder

**-i** (optional) case in-sensitive search

**searchpath** can be an absolute or a relative path

**filename** only filenames as plain string and no support for paths, subpaths, wildcards required
