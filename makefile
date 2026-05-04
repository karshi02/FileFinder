CC      = gcc

# ใช้ -O2 แต่ไม่ใช้ -flto (ลด temp files)
CFLAGS  = -Wall -Wextra -O2 -s -D_WIN32_WINNT=0x0600 -D_WIN32_IE=0x0600 -DWIN32_LEAN_AND_MEAN
LDFLAGS = -lshell32 -luser32 -lgdi32 -lole32 -luuid -lcomctl32 -s
OUT     = build/searcher.exe

# ไม่ใช้ -flto และไม่ใช้ mkdir -p ทุกครั้ง
SRC = main.c hotkey.c universal_scanner.c universal_ui.c frequent.c
OBJ = $(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(CFLAGS) $(LDFLAGS)
	$(STRIP) $(OUT) 2>/dev/null || true
	@echo "Build complete: $(OUT)"

# compile ทีละไฟล์
%.o: %.c universal_scanner.h hotkey.h universal_ui.h frequent.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)

run: $(OUT)
	./$(OUT)

.PHONY: all clean run
