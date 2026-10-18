# Data Structures — Member 1
# Data Structures Module

This folder contains the C implementation of data structures used by the Smart Photo Gallery.

## Structure

data-structures/
├── include/
│   └── photo_ds.h
└── src/
    └── photo_ds.c

## photo_ds.h

Defines all structures and function declarations.

Important structures:

- `PhotoRecord` — stores details of one photo
- `PhotoList` — linked list of all photos
- `PhotoQueue` — queue for photos waiting for archival
- `ActionStack` — stack for Undo Archive actions

## photo_ds.c

Contains the implementation of all data-structure operations.

## Concepts Used

### Linked List

Used to store photo records dynamically.

Functions include:

- Add photo
- Remove photo
- Find photo by ID
- Keep gallery records in memory

### Queue

Used to store photos waiting for archival.

It follows First-In-First-Out order.

### Stack

Used for Undo Archive.

It follows Last-In-First-Out order.

### Sorting

Photos can be sorted by:

- Name
- Date
- File size

This module is written in C for the Data Structures subject.
