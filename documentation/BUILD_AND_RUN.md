# Build and run the Smart Photo Gallery

## Required tools

- A C/C++ compiler
- CMake 3.16 or newer
- Qt 6 with the **Qt Widgets** component (Qt 5 Widgets also works)

The simplest route on macOS is to install Qt using the official Qt Online Installer and open this repository in **Qt Creator**. Select a Desktop Qt kit, then click Configure and Run.

## Build with CMake

Run these commands from the root project folder after Qt and CMake are installed:

```bash
cmake -S . -B build
cmake --build build
```

On macOS, the app is usually produced as `build/SmartPhotoGallery.app`. Open it with:

```bash
open build/SmartPhotoGallery.app
```

## Safe photo workflow

1. Click **Add Photos** and select images.
2. The application copies each selected photo into its own managed local storage. It never deletes the source image you selected.
3. Photos at least one year old can be archived.
4. Lossless archival happens only when the archive is smaller than the active managed file.
5. Opening an archived photo decompresses a temporary preview file. Closing the preview removes that temporary file.

For a compression demonstration, use a BMP or another uncompressed image. JPEG images are already compressed and often cannot be made smaller with lossless compression.
