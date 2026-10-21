cmake_minimum_required(VERSION 3.16)

project(SmartPhotoGallery VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(photo_data_structures
    data-structures/src/photo_ds.c
    data-structures/src/album_tree.c
)
target_include_directories(photo_data_structures PUBLIC data-structures/include)

add_library(photo_core
    backend/src/Photo.cpp
    backend/src/PhotoRepository.cpp
    photo-storage/src/StorageManager.cpp
)
target_include_directories(photo_core PUBLIC
    backend/include
    photo-storage/include
    data-structures/include
)
target_link_libraries(photo_core PUBLIC photo_data_structures Qt${QT_VERSION_MAJOR}::Widgets)

add_executable(SmartPhotoGallery
    frontend/src/main.cpp
    frontend/src/MainWindow.cpp
)
target_include_directories(SmartPhotoGallery PRIVATE
    frontend/src
    backend/include
    photo-storage/include
    data-structures/include
)
target_link_libraries(SmartPhotoGallery PRIVATE photo_core Qt${QT_VERSION_MAJOR}::Widgets)

if(APPLE)
    set_target_properties(SmartPhotoGallery PROPERTIES MACOSX_BUNDLE TRUE)
endif()
