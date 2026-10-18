# Backend — Member 2
# Backend Module

This folder contains the main C++ logic of the Smart Photo Gallery.

## Structure

backend/
├── include/
│   ├── Photo.h
│   └── PhotoRepository.h
└── src/
    ├── Photo.cpp
    └── PhotoRepository.cpp

## Photo.h and Photo.cpp

Defines the `Photo` class.

A Photo object stores and provides access to:

- Photo ID
- File name
- Date added
- Original size
- Current stored size
- Archive status
- One-year-old eligibility status

OOP concepts used:

- Class and object
- Constructor
- Encapsulation
- Member functions

## PhotoRepository.h and PhotoRepository.cpp

Defines the `PhotoRepository` class.

Its responsibilities are:

- Import selected photos
- Store photo records
- Search photos by ID
- Save metadata in `metadata.json`
- Load metadata when the application starts
- Remove photo records
- Sort photos by name, date, and size
- Calculate total managed storage

This class connects the C++ backend with the C linked-list data structure.
