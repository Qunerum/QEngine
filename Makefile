APP_NAME = QEngineApp
IGNORE_FILES = '*.c','*.h','*.asm','*.qsr'

CC    = gcc
ASM   = nasm
GLSLC = glslc

BUILD         = Build
BUILD_PROJECT = $(BUILD)/Project

OBJ_DIR = .obj
FILES   = Assets
FILES_PROJECT = $(BUILD_PROJECT)/$(FILES)

CFLAGS   = -Wall -Wextra -O3 -I. -Iinclude -Ilib -ffunction-sections -fdata-sections
ASMFLAGS = -f elf64
LDFLAGS  = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -Wl,--gc-sections -lasound -lpthread -lm

C_SRCS   = $(wildcard $(FILES)/*.c) $(wildcard Libs/*.c)
ASM_SRCS = $(wildcard $(FILES)/*.asm) $(wildcard Libs/*.asm)

C_OBJS   = $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(C_SRCS)))
ASM_OBJS = $(patsubst %.asm, $(OBJ_DIR)/%_asm.o, $(notdir $(ASM_SRCS)))
OBJS     = $(C_OBJS) $(ASM_OBJS)

vpath %.c Assets Libs
vpath %.asm Assets Libs

APP = $(BUILD_PROJECT)/$(APP_NAME)

all: build

prepare:
	@mkdir -p "$(OBJ_DIR)" "$(BUILD_PROJECT)" "$(FILES_PROJECT)"

build: prepare $(APP)

$(APP): $(OBJS)
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@echo "Compilation C $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%_asm.o: %.asm
	@echo "Assembling ASM $<..."
	@$(ASM) $(ASMFLAGS) $< -o $@

run: build
	@rsync -a --exclude={$(IGNORE_FILES)} $(FILES)/ $(FILES_PROJECT)/
	@cd $(BUILD_PROJECT) && ./$(APP_NAME)

clean:
	@rm -rf .obj $(BUILD)
	@echo "Cleaned!"

.PHONY: all prepare build run clean
