Smart Finder - Universal Search Tool

A powerful desktop search application that allows you to quickly find and launch files, folders, and applications using a global hotkey. The app learns from your usage patterns to show frequently used items first.

================================================================================
FEATURES
================================================================================

- Global Hotkey - Press Ctrl+Space anywhere to open the search window
- Smart Ranking - Frequently used items appear at the top of search results
- Real-time Indexing - Automatically indexes files on first run
- Fast Launch - Launch applications, open files, or folders directly from search
- Lightweight - Minimal resource usage, runs in background
- Persistent Database - Saves index and usage history for faster startup

================================================================================
REQUIREMENTS
================================================================================

- Windows 7 / 8 / 10 / 11 (64-bit)
- No additional dependencies required

================================================================================
INSTALLATION
================================================================================

Option 1: Installer (Recommended)
1. Download SmartFinderSetup.exe
2. Run the installer as Administrator
3. Follow the setup wizard
4. Program will start automatically after installation

Option 2: Portable Version
1. Download SmartFinder_Portable.zip
2. Extract to any folder
3. Run searcher.exe

================================================================================
USAGE
================================================================================

Basic Operation:
1. Open Search Window: Press Ctrl+Space anywhere
2. Type to Search: Start typing the name of the file, folder, or app
3. Navigate Results: Use Up and Down arrow keys
4. Launch Selected Item: Press Enter
5. Close Search: Press Esc

Search Tips:
- Search is case-insensitive
- Partial matches work (e.g., "expl" finds "explorer.exe")
- Results are ranked by usage frequency
- Recently launched items appear at the top

================================================================================
HOTKEYS SUMMARY
================================================================================

Ctrl+Space  - Open/close search window
Up / Down   - Navigate search results
Enter       - Launch selected item
Esc         - Close search window

================================================================================
FIRST RUN
================================================================================

On first launch, the application will:
1. Create a data folder in the installation directory
2. Scan and index files from common locations (Desktop, Documents, Downloads, Program Files, etc.)
3. Create a usage database to track frequently launched items

Note: Initial scanning may take 1-2 minutes depending on your system. Subsequent launches will be instant.

================================================================================
FILE STRUCTURE
================================================================================

SmartFinder/
+-- searcher.exe          # Main application
+-- data/                 # Data directory
    +-- universal.db      # Search index database
    +-- frequent.db       # Usage frequency database
    +-- config.ini        # Configuration file
+-- install.bat           # Portable installation script

================================================================================
CONFIGURATION
================================================================================

The application stores settings in data/config.ini (created automatically):

[Settings]
RunOnStartup=1           # 1=Enable, 0=Disable
StartMinimized=1         # Start in system tray
ShowNotifications=1      # Show bubble notifications

================================================================================
UNINSTALLATION
================================================================================

If installed via installer:
1. Open Control Panel -> Programs and Features
2. Find "Smart Finder"
3. Click Uninstall

For portable version:
- Simply delete the application folder

================================================================================
TROUBLESHOOTING
================================================================================

Hotkey doesn't work:
- Another application might be using Ctrl+Space
- Try restarting the application
- Check if the app is running (look for icon in system tray)

Search is slow:
- First run requires building the index - this is normal
- Subsequent searches should be instant
- If still slow, delete data/universal.db and restart (will re-index)

Application won't start:
- Check if it's already running (look in system tray)
- Try running as Administrator
- Check Windows Defender or antivirus isn't blocking it

Missing files in search results:
- The app indexes common locations by default
- Future versions may include custom path configuration

================================================================================
BUILDING FROM SOURCE
================================================================================

Prerequisites:
- MinGW GCC (MSYS2 UCRT64 recommended)
- Make utility

Build Steps:
cd /path/to/source
make clean
make all
./build/searcher.exe

On MSYS2 UCRT64:
pacman -S mingw-w64-ucrt-x86_64-gcc make

Required libraries (included in MinGW):
- shell32, user32, gdi32, ole32, uuid, comctl32

================================================================================
PERFORMANCE
================================================================================

- Memory usage: ~15-25 MB
- CPU usage: <1% when idle
- Index size: ~5-10 MB per 10,000 files
- Startup time: <1 second after initial index

================================================================================
KNOWN LIMITATIONS
================================================================================

- Initial indexing only covers common Windows locations
- No network drive support (local drives only)
- No real-time file system monitoring (requires manual re-index or restart)
- Command-line arguments not yet supported

================================================================================
VERSION HISTORY
================================================================================

v1.0.0 - Initial release
        - Global hotkey (Ctrl+Space)
        - Smart frequency-based ranking
        - Persistent database

================================================================================
LICENSE
================================================================================

Proprietary - All rights reserved

================================================================================
CONTACT
================================================================================

For issues or feature requests, please contact the developer.

================================================================================
CREDITS
================================================================================

Developed with C and Win32 API

================================================================================
Enjoy Smart Finder! Press Ctrl+Space to search everything instantly!
================================================================================
