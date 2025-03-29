# PowderDataProcessing

**Version 2.0**  
A Qt-based application for the automated processing of data from Fit2D.

Developed by Y. Filinchuk & Updated by Loïc Rochez – UCLouvain

---

## 🧪 Description

**PowderDataProcessing** is an intuitive graphical tool designed to efficiently analyze and transform experimental data related to X-ray powder diffraction.  
The available modules cover various tasks such as adding statistical uncertainties (sigmas), normalizing intensities, scaling data, and extracting scale factors from `.seq` files.

---

## 🖥️ Interface & Features

The application provides a simple graphical interface that allows you to:
- Select input/output files,
- Enter required parameters (e.g. detector-sample distance, 2Theta ranges, etc.),
- Start the processing with a single click.

### Available modules:
- **Add Sigmas**
- **Add Sigmas LEO**
- **Add Sigmas Series**
- **Normalize**
- **Scale Series**
- **Seq Scales**
- **Shorter Buffer**

👉 For a detailed explanation of each module, see the [`help.txt`](./help.txt) file.

---

## Installation

### 🪟 Windows

A precompiled executable is already included in the repository.  
Simply double-click on `PowderDataProcessing.exe` to launch the application.

---

### 🍏 macOS & 🐧 Linux

#### 🔧 Requirements:
- Qt6 (install via Homebrew or from the official site)
- CMake >= 3.5
- Xcode or any C++17-compatible compiler

#### 💡 Install Qt6 via Homebrew (macOS) or apt (Linux):
```bash
brew install qt
```

```bash
sudo apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools cmake build-essential
```
## ⚙️ Compilation
### 🍏 macOS
```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
make
```

### 🐧 Linux
```bash
mkdir build
cd build
cmake ..
make
```
