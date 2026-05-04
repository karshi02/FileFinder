CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -D_WIN32_WINNT=0x0600 -D_WIN32_IE=0x0600
LDFLAGS = -mwindows -lshell32 -luser32 -lgdi32 -lole32 -luuid -lcomctl32 -lcomdlg32
OUT     = build/searcher.exe

SRC = main.c hotkey.c universal_scanner.c universal_ui.c frequent.c settings.c
OBJ = $(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	mkdir -p build
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)
	@echo ""
	@echo "=========================================="
	@echo "  Smart Finder — Build OK"
	@echo "  Run: build/searcher.exe"
	@echo "=========================================="

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)
	@echo "Cleaned."

# รัน debug version (มี console)
run-debug: CFLAGS += -g
run-debug: LDFLAGS = -lshell32 -luser32 -lgdi32 -lole32 -luuid -lcomctl32 -lcomdlg32
run-debug: $(OUT)
	./$(OUT)

run: $(OUT)
	start "" "$(OUT)"

rebuild: clean all

# สร้าง installer ด้วย NSIS
installer: $(OUT)
	@echo "Building installer..."
	makensis installer.nsi
	@echo "SmartFinderSetup.exe created!"

.PHONY: all clean rebuild run run-debug installer
