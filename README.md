# Smart Finder

**Universal App Launcher for Windows** — กด hotkey ได้ทุกที่ ค้นหาและเปิดแอพได้ทันที

---

## ภาพรวม

Smart Finder เป็นโปรแกรม background ที่รอรับ hotkey แล้วแสดง popup ค้นหาแอพและไฟล์บน Windows โดยไม่ต้องเปิด Start Menu หรือ Explorer

```
กด Ctrl+Alt+Space  →  popup ขึ้น  →  พิมพ์ชื่อแอพ  →  Enter เพื่อเปิด
```

---

## Features

| Feature | รายละเอียด |
|---------|-----------|
| **Global Hotkey** | กด hotkey ได้จากทุก window ที่กำลังใช้งาน |
| **App Icons** | แสดงไอคอนจริงของแต่ละแอพใน popup |
| **Frequent Apps** | แอพที่ใช้บ่อยขึ้นมาก่อนอัตโนมัติ |
| **Fast Search** | ผลลัพธ์อัปเดตทันทีขณะพิมพ์ |
| **Run at Startup** | ตั้งให้รันอัตโนมัติเมื่อเปิด Windows ได้ |
| **Custom Hotkey** | เปลี่ยน hotkey ได้ตามต้องการผ่าน Settings |
| **System Tray** | รันเบื้องหลังแบบ tray icon ไม่รกหน้าจอ |
| **Index Cache** | scan ครั้งแรกเท่านั้น ครั้งต่อไปโหลด cache เร็วมาก |

---

## การใช้งาน

### เปิด Popup
- กด **Ctrl+Alt+Space** (default) จากทุกที่
- หรือ **คลิกซ้าย** ที่ tray icon มุมขวาล่าง
- หรือ **คลิกขวา → Open Search**

### ภายใน Popup
| ปุ่ม | หน้าที่ |
|------|--------|
| พิมพ์ตัวอักษร | ค้นหาแอพ |
| `↑` `↓` | เลื่อนเลือก |
| `Enter` | เปิดแอพที่เลือก |
| `ESC` | ปิด popup |
| `Backspace` | ลบตัวอักษร |

### Settings
คลิกขวาที่ tray icon → **Settings**

| ตัวเลือก | หน้าที่ |
|----------|--------|
| Hotkey | กด key combo ที่ต้องการเพื่อเปลี่ยน |
| Run at Startup | เปิด/ปิด การรันอัตโนมัติตอนเปิด Windows |
| Show recent apps | เปิด/ปิด แสดงแอพล่าสุดตอนเปิด popup |
| Close popup after launch | เปิด/ปิด ปิด popup หลังกด Enter |
| Max results | จำนวนผลลัพธ์สูงสุด (5–50) |

---

## Installation

### วิธีที่ 1 — Setup.exe (แนะนำ)
```
1. รัน SmartFinderSetup.exe
2. โปรแกรมติดตั้งและรันอัตโนมัติ
3. กด Ctrl+Alt+Space เพื่อเริ่มใช้งาน
```

### วิธีที่ 2 — Portable
```
1. copy build\searcher.exe ไปไว้ที่ใดก็ได้
2. ดับเบิลคลิกเพื่อรัน
3. ข้อมูลจะถูกบันทึกใน data\ ในโฟลเดอร์เดียวกัน
```

### วิธีที่ 3 — Build จาก Source
```bash
# ต้องการ MSYS2 + MinGW-w64
make          # build
make run      # รัน
make installer  # สร้าง Setup.exe (ต้องการ NSIS)
```

---

## โครงสร้างโปรเจกต์

```
smart-finder/
├── main.c                # entry point, tray icon, message loop
├── hotkey.c/.h           # global hotkey registration (auto-fallback)
├── universal_scanner.c/.h # scan Start Menu, index, search
├── universal_ui.c/.h     # popup window, owner-draw listbox, icons
├── frequent.c/.h         # บันทึกและเรียงแอพที่ใช้บ่อย
├── settings.c/.h         # โหลด/บันทึก settings + dialog
├── makefile              # build system
├── installer.nsi         # NSIS installer script
├── build_release.bat     # one-click build + package
└── data/
    ├── universal.db      # index cache (binary)
    ├── frequent.db       # usage stats (binary)
    └── settings.ini      # user settings
```

---

## System Requirements

- Windows 7 / 8 / 10 / 11 (64-bit)
- RAM ~10 MB
- Disk ~5 MB
- ไม่ต้องติดตั้ง runtime เพิ่มเติม

---

## Build Requirements

- [MSYS2](https://www.msys2.org/) with UCRT64
- `pacman -S mingw-w64-ucrt-x86_64-gcc`
- [NSIS](https://nsis.sourceforge.io/) (สำหรับสร้าง installer เท่านั้น)

---

## Troubleshooting

**Hotkey ไม่ทำงาน**
Hotkey อาจชนกับโปรแกรมอื่น (เช่น VS Code ใช้ Ctrl+Space) คลิกขวา tray → Settings → เปลี่ยน hotkey

**Popup ไม่ขึ้น**
คลิกซ้ายที่ tray icon หรือ คลิกขวา → Open Search แทนได้

**แอพที่ติดตั้งใหม่ไม่ขึ้น**
ลบ `data\universal.db` แล้วรันใหม่ โปรแกรมจะ scan ใหม่อัตโนมัติ

**โปรแกรมรันซ้ำไม่ได้**
โปรแกรมมี mutex ป้องกัน ถ้าเห็น popup แจ้ง "กำลังทำงานอยู่แล้ว" ให้ดูที่ System Tray

---

## License

MIT License — ใช้งานได้อิสระ แก้ไขได้


---

## Features

| Feature | รายละเอียด |
|---------|-----------|
| **Global Hotkey** | กด hotkey ได้จากทุก window ที่กำลังใช้งาน |
| **App Icons** | แสดงไอคอนจริงของแต่ละแอพใน popup |
| **Frequent Apps** | แอพที่ใช้บ่อยขึ้นมาก่อนอัตโนมัติ |
| **Fast Search** | ผลลัพธ์อัปเดตทันทีขณะพิมพ์ |
| **Run at Startup** | ตั้งให้รันอัตโนมัติเมื่อเปิด Windows ได้ |
| **Custom Hotkey** | เปลี่ยน hotkey ได้ตามต้องการผ่าน Settings |
| **System Tray** | รันเบื้องหลังแบบ tray icon ไม่รกหน้าจอ |
| **Index Cache** | scan ครั้งแรกเท่านั้น ครั้งต่อไปโหลด cache เร็วมาก |

---

## การใช้งาน

### เปิด Popup
- กด **Ctrl+Alt+Space** (default) จากทุกที่
- หรือ **คลิกซ้าย** ที่ tray icon มุมขวาล่าง
- หรือ **คลิกขวา → Open Search**

### ภายใน Popup
| ปุ่ม | หน้าที่ |
|------|--------|
| พิมพ์ตัวอักษร | ค้นหาแอพ |
| `↑` `↓` | เลื่อนเลือก |
| `Enter` | เปิดแอพที่เลือก |
| `ESC` | ปิด popup |
| `Backspace` | ลบตัวอักษร |

### Settings
คลิกขวาที่ tray icon → **Settings**

| ตัวเลือก | หน้าที่ |
|----------|--------|
| Hotkey | กด key combo ที่ต้องการเพื่อเปลี่ยน |
| Run at Startup | เปิด/ปิด การรันอัตโนมัติตอนเปิด Windows |
| Show recent apps | เปิด/ปิด แสดงแอพล่าสุดตอนเปิด popup |
| Close popup after launch | เปิด/ปิด ปิด popup หลังกด Enter |
| Max results | จำนวนผลลัพธ์สูงสุด (5–50) |

---

## Installation

### วิธีที่ 1 — Setup.exe (แนะนำ)

v1.0.1 (2026)
เพิ่ม auto-fallback hotkey เมื่อ key ซ้ำกับโปรแกรมอื่น

แก้ไข popup ไม่แสดงบน Windows 11

ปรับปรุง performance การโหลด index

เพิ่ม Settings dialog