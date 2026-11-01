CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -g -O2
RAYLIB ?= .deps/raylib
CPPFLAGS = -Isrc
ifneq ($(wildcard $(RAYLIB)/include/raylib.h),)
CPPFLAGS += -I$(RAYLIB)/include
LDLIBS = -L$(RAYLIB)/lib -Wl,-rpath,'$$ORIGIN/../.deps/raylib/lib' -lraylib -lm
else
# Preserve the group's /usr/local Raylib installation when no local bundle exists.
CPPFLAGS += -I/usr/local/include
LDLIBS = -L/usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif
CORE = src/world.c src/collision.c
HEADERS = $(wildcard src/*.h)

.PHONY: all run clean test masks sanitize
all: build/vhs

build/vhs: src/main.c src/scene_draw.c $(CORE) $(HEADERS)
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/scene_draw.c $(CORE) $(LDLIBS) -o $@

run: build/vhs
	./build/vhs

masks:
	python3 tools/build_masks.py

build/test_rooms: tests/test_rooms.c tests/navigation.h $(CORE) $(HEADERS)
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -Itests tests/test_rooms.c $(CORE) $(LDLIBS) -o $@

test: build/test_rooms
	python3 tests/test_assets.py
	./build/test_rooms

sanitize:
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -Itests -O1 -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_rooms.c $(CORE) $(LDLIBS) -o build/test_sanitize
	ASAN_OPTIONS=detect_leaks=1 ./build/test_sanitize

clean:
	rm -f build/vhs build/test_rooms build/test_sanitize build/test_routes
