╔═══════════════════════════════════════════════════════════════════════════════╗
║                           SMART FINDER - Universal Search Tool                ║
║                           Version 1.0.0 | Windows 64-bit                      ║
╚═══════════════════════════════════════════════════════════════════════════════╝

┌───────────────────────────────────────────────────────────────────────────────┐
│ DESCRIPTION                                                                   │
└───────────────────────────────────────────────────────────────────────────────┘

  A fast desktop search application that allows you to instantly find and launch
  files, folders, and applications using a global hotkey.

┌───────────────────────────────────────────────────────────────────────────────┐
│ FEATURES                                                                      │
└───────────────────────────────────────────────────────────────────────────────┘

  ⚡ Global Hotkey      →  Press Ctrl+Space anywhere to open search
  📊 Usage-Based Ranking →  Frequently used items appear on top
  🔍 Fast Search        →  Results update instantly as you type
  💾 Persistent Database →  Saves index and usage history
  🪶 Lightweight        →  Minimal memory and CPU usage
  🚀 Auto-Start         →  Can run on Windows startup

┌───────────────────────────────────────────────────────────────────────────────┐
│ SYSTEM REQUIREMENTS                                                           │
└───────────────────────────────────────────────────────────────────────────────┘

  ▪ Windows 7 / 8 / 10 / 11 (64-bit)
  ▪ No additional dependencies required
  ▪ ~20 MB free disk space

┌───────────────────────────────────────────────────────────────────────────────┐
│ INSTALLATION                                                                  │
└───────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ Option 1: Installer (Recommended)                                          │
  └─────────────────────────────────────────────────────────────────────────────┘

    1. Run FinderSetup.exe as Administrator
    2. Follow the setup wizard
    3. Program starts automatically after installation

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ Option 2: Portable Version                                                 │
  └─────────────────────────────────────────────────────────────────────────────┘

    1. Extract Finder_Portable.zip to any folder
    2. Double-click searcher.exe to run

┌───────────────────────────────────────────────────────────────────────────────┐
│ QUICK START                                                                   │
└───────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │                                                                             │
  │    1. Press Ctrl+Space → Search window opens                                │
  │    2. Start typing      → Results appear instantly                         │
  │    3. Use ↑/↓ arrows    → Navigate through results                         │
  │    4. Press Enter       → Launch selected item                             │
  │    5. Press Esc         → Close search window                              │
  │                                                                             │
  └─────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ HOTKEY REFERENCE                                                              │
└───────────────────────────────────────────────────────────────────────────────┘

  ┌──────────────────┬────────────────────────────────────────────────────────┐
  │ HOTKEY           │ ACTION                                                 │
  ├──────────────────┼────────────────────────────────────────────────────────┤
  │ Ctrl + Space     │ Open / Close search window                             │
  │ ↑ (Up Arrow)     │ Move up in search results                              │
  │ ↓ (Down Arrow)   │ Move down in search results                            │
  │ Enter            │ Launch selected file / app / folder                    │
  │ Esc              │ Close search window                                    │
  └──────────────────┴────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ SEARCH TIPS                                                                   │
└───────────────────────────────────────────────────────────────────────────────┘

  💡 Case-insensitive search     → "EXPLORER" = "explorer"
  💡 Partial keywords work       → "expl" finds "explorer.exe"
  💡 Frequently used items       → Appear higher in results
  💡 Instant filtering           → Results update as you type

┌───────────────────────────────────────────────────────────────────────────────┐
│ FIRST RUN                                                                     │
└───────────────────────────────────────────────────────────────────────────────┘

  On first launch, the application will automatically:

  ┌────────────────────────────────────────────────────────────────────────────┐
  │  ✓ Create data folder in installation directory                           │
  │  ✓ Scan common locations (Desktop, Documents, Downloads, Program Files)   │
  │  ✓ Build search index database                                            │
  │  ✓ Initialize usage tracking database                                     │
  └────────────────────────────────────────────────────────────────────────────┘

  ⚠ Note: Initial scanning may take 1-2 minutes depending on your system.
         Subsequent launches will be instant.

┌───────────────────────────────────────────────────────────────────────────────┐
│ FILE STRUCTURE                                                                │
└───────────────────────────────────────────────────────────────────────────────┘

  📁 Finder/
  │
  ├── 📄 searcher.exe      → Main application
  │
  ├── 📁 data/
  │   ├── 🗄️ universal.db  → Search index database
  │   ├── 📊 frequent.db   → Usage frequency database
  │   └── ⚙️ config.ini    → Configuration file
  │
  └── 📜 install.bat       → Portable installation script

┌───────────────────────────────────────────────────────────────────────────────┐
│ CONFIGURATION                                                                 │
└───────────────────────────────────────────────────────────────────────────────┘

  Edit data/config.ini to customize settings:

  ┌────────────────────────────────────────────────────────────────────────────┐
  │  [Settings]                                                               │
  │  RunOnStartup=1        → 1=Enable, 0=Disable auto-start                   │
  │  StartMinimized=1      → 1=Start to tray, 0=Normal window                 │
  │  ShowNotifications=1   → 1=Show bubble alerts, 0=Silent                    │
  └────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ UNINSTALLATION                                                                │
└───────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ Via Installer:                                                             │
  │   Control Panel → Programs and Features → Smart Finder → Uninstall        │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ Portable Version:                                                          │
  │   Simply delete the application folder                                     │
  └─────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ TROUBLESHOOTING                                                               │
└───────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ ❌ Hotkey doesn't work                                                     │
  │    ▪ Another app may be using Ctrl+Space                                   │
  │    ▪ Restart the application                                               │
  │    ▪ Check if program is running (look for icon in system tray)           │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ ❌ Slow search performance                                                 │
  │    ▪ First run requires building index - this is normal                    │
  │    ▪ Delete data/universal.db and restart to rebuild index                 │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ ❌ Application won't start                                                 │
  │    ▪ Check if already running (system tray)                                │
  │    ▪ Try Run as Administrator                                              │
  │    ▪ Check Windows Defender/Antivirus                                      │
  └─────────────────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────────────────┐
  │ ❌ Files not appearing in search                                           │
  │    ▪ App indexes common locations by default                               │
  │    ▪ Try deleting data/universal.db and restart to re-index               │
  └─────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ BUILDING FROM SOURCE                                                          │
└───────────────────────────────────────────────────────────────────────────────┘

  Prerequisites:
  ▪ MinGW GCC (MSYS2 UCRT64 recommended)
  ▪ Make utility

  Commands:
  ┌────────────────────────────────────────────────────────────────────────────┐
  │  $ make clean          # Remove previous builds                           │
  │  $ make all            # Compile the application                          │
  │  $ make run            # Build and run                                    │
  │  $ ./build/searcher.exe # Run directly                                    │
  └────────────────────────────────────────────────────────────────────────────┘

  On MSYS2 UCRT64:
  ┌────────────────────────────────────────────────────────────────────────────┐
  │  $ pacman -S mingw-w64-ucrt-x86_64-gcc make                               │
  └────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ PERFORMANCE                                                                   │
└───────────────────────────────────────────────────────────────────────────────┘

  ┌────────────────────────────────────────────────────────────────────────────┐
  │  Memory usage     →  ~15-25 MB                                            │
  │  CPU usage        →  <1% when idle                                        │
  │  Index size       →  ~5-10 MB per 10,000 files                            │
  │  Startup time     →  <1 second after initial index                        │
  └────────────────────────────────────────────────────────────────────────────┘

┌───────────────────────────────────────────────────────────────────────────────┐
│ KNOWN LIMITATIONS                                                             │
└───────────────────────────────────────────────────────────────────────────────┘

  ▪ Local drives only (no network drive support)
  ▪ No real-time file system monitoring
  ▪ Limited to common Windows locations
  ▪ Command-line arguments not supported yet

┌───────────────────────────────────────────────────────────────────────────────┐
│ VERSION HISTORY                                                               │
└───────────────────────────────────────────────────────────────────────────────┘

  v1.0.0 (2024)
  ├── Initial release
  ├── Global hotkey (Ctrl+Space)
  ├── Smart frequency-based ranking
  ├── Persistent database
  └── System tray support

┌───────────────────────────────────────────────────────────────────────────────┐
│ LICENSE                                                                       │
└───────────────────────────────────────────────────────────────────────────────┘

  Proprietary - All rights reserved

┌───────────────────────────────────────────────────────────────────────────────┐
│ CREDITS                                                                       │
└───────────────────────────────────────────────────────────────────────────────┘

  Developed with C and Win32 API

╔═══════════════════════════════════════════════════════════════════════════════╗
║                                                                               ║
║        Press Ctrl+Space and start searching everything instantly!            ║
║                                                                               ║
╚═══════════════════════════════════════════════════════════════════════════════╝