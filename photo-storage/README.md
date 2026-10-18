# Photo Storage — Member 4
# Photo Storage and Compression Module

This folder manages archival compression, restoration, temporary decompression, and managed photo storage.

## Structure

photo-storage/
├── include/
│   └── StorageManager.h
└── src/
    └── StorageManager.cpp

## StorageManager.h

Defines the `StorageManager` class.

It declares functions for:

- Checking photo age
- Archiving a photo
- Restoring an archived photo
- Creating temporary preview files
- Deleting managed photo files
- Managing archive storage

## StorageManager.cpp

Implements the complete archive and restore process.

## Archive Workflow

Active managed photo
        ↓
Check photo age or manual archive request
        ↓
Apply lossless compression
        ↓
Store compressed file as .spg archive
        ↓
Remove managed active copy
        ↓
Update status to Archived

## Preview Workflow

User selects archived photo
        ↓
Archive is temporarily decompressed
        ↓
Temporary image file is created
        ↓
Photo is displayed
        ↓
Temporary file is deleted after preview closes

## Safety

The original photo selected by the user is never deleted.

The app first makes its own managed copy. Archive and Delete operations affect only that managed copy.

## Limitation

JPEG images are already compressed, so lossless compression may not save much storage. BMP images are better for demonstrating compression.
