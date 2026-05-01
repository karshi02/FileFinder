CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
LDFLAGS = -lshell32 -luser32 -lgdi32 -lole32 -luuid
OUT     = build/searcher.exe

SRC = main.c hotkey.c universal_scanner.c universal_ui.c frequent.c
OBJ = $(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	mkdir -p build
	$(CC) $(OBJ) -o $(OUT) $(CFLAGS) $(LDFLAGS)
	@echo ""
	@echo "=========================================="
	@echo "  Universal Search with Frequent Apps"
	@echo "  Ready! Press Ctrl+Space to search"
	@echo "=========================================="

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)
	@echo "Cleaned."

run: $(OUT)
	./$(OUT)

rebuild: clean all

.PHONY: all clean rebuild run
