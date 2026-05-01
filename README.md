SMART FILE FINDER (HOTKEY VERSION) — FULL SYSTEM DESIGN (LOCAL ONLY)

========================================

1. OVERVIEW
   ========================================
   โปรแกรมค้นหาไฟล์ในเครื่อง (Local Application)

* ทำงานเบื้องหลัง (Background)
* เรียกใช้งานด้วย Hotkey (เช่น Ctrl + Space)
* ไม่มีการลบ/แก้ไขไฟล์ใด ๆ ทั้งสิ้น
* มีหน้าที่ “ค้นหาและเปิดไฟล์เท่านั้น”

========================================
2. SYSTEM GOAL
==============

* กดปุ่ม → popup ค้นหาขึ้นทันที
* พิมพ์แบบจำลาง ๆ → เจอไฟล์
* ใช้งานเร็ว ไม่ต้องเปิดโปรแกรมก่อน

========================================
3. PROJECT STRUCTURE
====================

smart-finder/
│
├── main.c                # entry + loop หลัก
├── hotkey.c              # จัดการ global hotkey
├── hotkey.h
│
├── scanner.c             # scan ไฟล์ในเครื่อง
├── scanner.h
│
├── indexer.c             # เก็บข้อมูลไฟล์
│
├── indexer.h
├── search.c              # ค้นหาไฟล์
├── search.h
│
├── ranking.c             # จัดอันดับผลลัพธ์
├── ranking.h
│
├── ui.c                  # popup / input / แสดงผล
├── ui.h
│
├── utils.c               # string / helper
├── utils.h
│
├── data/
│   └── index.db          # cache ไฟล์
│
├── Makefile              # build ทั้งระบบ
├── run.bat               # รันโปรแกรม
│
└── build/
└── finder.exe

========================================
4. MAKEFILE (BUILD SYSTEM)
==========================

CC = gcc
CFLAGS = -Wall
OUT = build/finder.exe

SRC = main.c hotkey.c scanner.c indexer.c search.c ranking.c ui.c utils.c

all:
$(CC) $(SRC) -o $(OUT) $(CFLAGS)

clean:
del build\finder.exe

========================================
5. RUN FILE (run.bat)
=====================

@echo off
build\finder.exe

========================================
6. CORE DATA STRUCTURE
======================

typedef struct {
char name[256];
char path[1024];
long last_access;
int score;
} File;

========================================
7. SYSTEM FLOW
==============

[START PROGRAM]
→ โหลด index.db
→ ถ้าไม่มี → scan ทั้งเครื่อง
→ เข้าสู่ background loop

[WAIT]
→ รอ hotkey

[HOTKEY PRESSED]
→ เปิด popup

[USER INPUT]
→ parse keyword

[SEARCH]
→ filter files
→ calculate score
→ sort

[RESULT]
→ แสดง top results

[USER SELECT]
→ เปิดไฟล์ (read-only)

========================================
8. MODULE BEHAVIOR
==================

8.1 HOTKEY

* RegisterHotKey()
* ตรวจจับ key แบบ global

8.2 SCANNER

* recursive scan directory
* ใช้ FindFirstFile()

8.3 INDEXER

* เก็บไฟล์ใน memory
* save ลง index.db

8.4 SEARCH

* ใช้ strstr() หรือ fuzzy match

8.5 RANKING
score =
+50 (ชื่อไฟล์ตรง)
+30 (ใช้ล่าสุด)
+20 (type ตรง)

8.6 UI

* popup input
* แสดงรายการ
* Enter = เปิดไฟล์
* Esc = ปิด

========================================
9. RULES / CONSTRAINTS
======================

* ❌ ห้ามลบไฟล์
* ❌ ห้ามแก้ไขไฟล์
* ❌ ห้ามเขียนทับไฟล์ผู้ใช้
* ✅ อ่าน metadata ได้
* ✅ เปิดไฟล์ได้ (read-only)

========================================
10. PERFORMANCE REQUIREMENTS
============================

* search < 0.1 วินาที
* รองรับ 10,000–100,000 ไฟล์
* RAM ใช้ไม่เกิน ~200MB

========================================
11. DESIGN CONCEPT
==================

* “Memory-based search”
* ผู้ใช้ไม่ต้องจำชื่อไฟล์
* เน้น speed + simplicity
* ใช้ index แทน scan ทุกครั้ง

========================================
12. COMMON BUGS + FIX
=====================

12.1 PATH TOO LONG
ปัญหา:

* crash

แก้:

* ใช้ buffer 1024+
* ตรวจความยาวก่อน copy

---

12.2 INFINITE RECURSION
ปัญหา:

* scan ไม่จบ

แก้:

* skip symlink / junction

---

12.3 MEMORY OVERFLOW
ปัญหา:

* crash เมื่อไฟล์เยอะ

แก้:

* malloc + realloc
* จำกัดจำนวนไฟล์

---

12.4 HOTKEY NOT WORK
ปัญหา:

* key ไม่ทำงาน

แก้:

* เปลี่ยน key
* run as admin

---

12.5 UI NOT FOCUS
ปัญหา:

* พิมพ์ไม่ได้

แก้:

* SetForegroundWindow()

---

12.6 SEARCH SLOW
ปัญหา:

* lag

แก้:

* ใช้ index
* ลด loop

---

12.7 THAI TEXT ISSUE
ปัญหา:

* หาไม่เจอ

แก้:

* UTF-8
* normalize string

========================================
13. DEBUG METHOD
================

* printf debug ทุก step
* log file count
* test module แยกกัน:

  * scanner
  * search
  * ranking

========================================
14. DEVELOPMENT STEPS (MVP)
===========================

STEP 1:

* scan + เก็บไฟล์

STEP 2:

* search ชื่อไฟล์

STEP 3:

* ranking

STEP 4:

* hotkey

STEP 5:

* popup UI

========================================
15. FINAL SUMMARY
=================

ระบบนี้คือ:

* โปรแกรม background
* เรียกด้วย hotkey
* ค้นหาไฟล์อย่างเดียว
* ไม่ยุ่งกับไฟล์อื่น

# END
