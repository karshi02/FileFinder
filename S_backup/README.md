<div align="center">

# 🔍 Universal Search for Windows

### *Find anything on your PC in an instant — just like macOS Spotlight*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Windows](https://img.shields.io/badge/Windows-10%2B-0078D6.svg?logo=windows)](https://www.microsoft.com/windows)
[![Made with C](https://img.shields.io/badge/Made%20with-C-00599C.svg?logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)]()

</div>

---

## 📌 Overview

**Universal Search for Windows** is a lightweight, lightning-fast search tool that brings **Spotlight-like search** to Windows. Press `Ctrl+Space`, type what you're looking for, and launch it instantly — no more digging through Start Menu or File Explorer.

> ⚡ **Zero bloat. Zero tracking. Pure speed.**

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🚀 **Instant Search** | Results appear as you type — no lag |
| 📱 **App Launcher** | Find and launch any installed application |
| 📁 **File Finder** | Search through documents, executables, and recent files |
| 🔥 **Smart Learning** | Frequently used apps rise to the top automatically |
| ⌨️ **Keyboard First** | Full keyboard navigation — mouse optional |
| 🖱️ **Mouse Support** | Double-click works too |
| 💾 **Persistent Cache** | First scan builds index, subsequent launches are instant |
| 🔒 **Privacy Focused** | 100% offline — no data sent anywhere |
| 📦 **Portable** | Single executable, no installation required |

---

## 🎮 Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl` + `Space` | Open / Close search window |
| `↓` (Down Arrow) | Move to next result |
| `↑` (Up Arrow) | Move to previous result |
| `Enter` | Launch selected app / file |
| `ESC` | Close search window |

> 💡 **Tip:** You can also double-click any result with your mouse!

---

## 📦 What You Can Search

| Category | Examples |
|----------|----------|
| 🖥️ **Applications** | Chrome, VS Code, Spotify, Photoshop... |
| ⚙️ **Programs** | Any `.exe` from Program Files |
| 📄 **Recent Files** | Last opened documents |
| 📁 **User Files** | Files from `C:\Users` |
| 🎯 **System Tools** | Command Prompt, PowerShell, Task Manager |

---

## 🚀 Quick Start

### Prerequisites
- Windows 10 or Windows 11
- GCC (MinGW-w64) — for building from source

### Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/universal-search-windows.git
cd universal-search-windows

# Build the application
make

# Run it!
./build/searcher.exe
