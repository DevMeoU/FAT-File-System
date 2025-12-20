# FAT File System Implementation

[![Language](https://img.shields.io/badge/Language-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Build System](https://img.shields.io/badge/Build-Makefile-green.svg)](https://www.gnu.org/software/make/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()
[![License](https://img.shields.io/badge/License-MIT-orange.svg)]()

[🇻🇳 **Vietnamese Version / Phiên bản Tiếng Việt**](Readme_vi.md)

---

## 📖 Abstract

This project is a userspace C implementation of the **FAT12 file system**, designed to run on Windows/Linux. It simulates low-level hardware interactions by treating a raw disk image (`floppy.img`) as a physical device.

The primary goal is to demonstrate **driver architecture**, **layered design patterns**, and **raw filesystem manipulation** without relying on the host OS's filesystem drivers.

---

## 📑 Table of Contents
- [Architecture](#-architecture)
  - [Layered Design](#layered-design)
  - [Data Flow](#data-flow)
- [Project Structure](#-project-structure)
- [Component Details](#-component-details)
- [Building & Running](#-building--running)
- [Screenshots](#-screenshots)

---

## 🏗 Architecture

### Layered Design
The project follows a strict layered architecture to ensure modularity and separation of concerns.

```mermaid
graph TD
    User([User]) <--> App[Application Layer]
    App <--> MW[Middleware Layer]
    MW <--> FAT[FAT File System Driver]
    FAT <--> HAL[Hardware Abstraction Layer]
    HAL <--> IP[IP Driver (Low-Level I/O)]
    IP <--> IMG[(floppy.img)]
```

### Data Flow
1.  **Application**: User requests a file (e.g., `cat file.txt`).
2.  **Middleware**: Validates request and orchestrates the operation.
3.  **FAT Driver**: Parses the FAT table and Root Directory to find clusters.
4.  **HAL**: Converts logical interactions (sectors) into physical offsets.
5.  **IP Driver**: Performs raw `fseek`/`fread` on the disk image.

---

## 📂 Project Structure

The source code is organized into logical modules within the `src` directory:

```text
Project Root
├── 📁 build/              # Generated build artifacts
├── 📁 images/             # Disk images (floppy.img)
├── 📁 src/
│   ├── 📁 application/    # CLI & User Interface
│   ├── 📁 middleware/     # Business logic & Data processing
│   ├── 📁 fat_driver/     # FAT12 logic (BootSector, FAT, RootDir)
│   ├── 📁 hal/            # Sector-to-Offset translation
│   ├── 📁 ip_driver/      # Raw File I/O (fseek, fread)
│   ├── 📁 common/         # Shared types (integers, status codes)
│   └── 📁 utilities/      # Data structures (LinkedList, Logger)
├── CMakeLists.txt         # CMake build configuration
├── Makefile               # GNU Make configuration
└── README.md              # This file
```

---

## 🔧 Component Details

<details>
<summary><b>1. IP Driver (Low-Level I/O)</b></summary>

The interface to the "physical" storage. It strictly handles byte-level operations.
*   **Role**: Open/Close `floppy.img`, Read/Write raw bytes at offsets.
*   **Key API**: `ip_driver_read`, `ip_driver_write`.
</details>

<details>
<summary><b>2. Hardware Abstraction Layer (HAL)</b></summary>

Translates filesystem concepts (sectors) into storage concepts (offsets).
*   **Role**: Sector addressing (LBA).
*   **Operation**: `Offset = Sector_Index * Sector_Size`.
</details>

<details>
<summary><b>3. FAT Driver</b></summary>

The core filesystem logic. It understands the disk structure.
*   **Role**: Parsing Boot Sector, navigating the FAT chains, and reading directory entries.
*   **Features**: Long File Name (LFN) support (limited), Cluster chaining.
</details>

<details>
<summary><b>4. Middleware & Application</b></summary>

*   **Middleware**: Bridges the raw data from the driver to the application (e.g., converting buffer to string, error handling).
*   **Application**: A CLI wrapper allowing users to interact with the system (commands like `ls`, `cat`).
</details>

---

## 🚀 Building & Running

### Prerequisites
*   **GCC** (MinGW for Windows or native GCC for Linux)
*   **Make**

### Compilation

You can build the project using the provided `Makefile`:

```bash
# Build the release version (Optimized)
make release

# Build the debug version (with symbols)
make debug

# Clean build artifacts
make clean
```

### Execution

Run the application pointing to your disk image:

```bash
# Run with default settings
make run
```

Or manually:
```bash
./build/bin/main.exe images/floppy.img
```

---

## 📸 Screenshots

| Directory Listing (`ls`) | File Content (`cat`) |
|:-------------------------:|:--------------------:|
| ![Listing](image/README/1742297625445.png) | ![Reading](image/README/1742297655626.png) |

**Detailed View:**
![Detailed View](image/README/1742297708543.png)

---

## 📜 License
This project is open-source and available under the terms of the **MIT License**.
