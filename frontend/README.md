# Frontend — Member 3
# Frontend Module

This folder contains the Qt Widgets graphical interface of the Smart Photo Gallery.

## Structure

frontend/
├── resources/
│   └── Future icons, images, and style sheets
└── src/
    ├── main.cpp
    ├── MainWindow.h
    └── MainWindow.cpp

## main.cpp

This is the application entry point.

It:

- Starts the Qt application
- Sets application information
- Creates the MainWindow object
- Starts the GUI event loop

## MainWindow.h

Declares the `MainWindow` class.

MainWindow inherits from Qt's `QMainWindow`.

It contains:

- PhotoRepository object
- StorageManager object
- Undo Archive stack
- Search box
- Photo table
- Storage summary cards

## MainWindow.cpp

Creates and controls the complete user interface.

## Features

- Add Photos
- Archive Selected
- Archive Old Photos
- Preview
- Undo Archive
- Delete
- Search bar
- Photo table
- Photo count
- Managed storage count
- Eligible-for-archive count

## OOP Concepts

- Inheritance: `MainWindow` inherits from `QMainWindow`
- Composition: MainWindow contains backend and storage-manager objects
- Event-driven programming: buttons trigger functions through Qt signals and slots
