APP_NAME = QEngineApp
IGNORE_FILES = '*.c','*.h','*.asm','*.qbc','*.qsr','*.qfr'

CC    = gcc
ASM   = nasm
GLSLC = glslc

BUILD         = Build
BUILD_ENGINE  = $(BUILD)/Engine
BUILD_PROJECT = $(BUILD)/Project

OBJ_EDITOR = .obj/editor
OBJ_GAME   = .obj/game

FILES         = Assets
FILES_PROJECT = $(BUILD_PROJECT)/$(FILES)

CFLAGS   = -Wall -Wextra -O2 -I. -Iinclude -Ilib -ffunction-sections -fdata-sections
ASMFLAGS = -f elf64
LDFLAGS  = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -Wl,--gc-sections -lasound -lpthread -lm

C_SRCS   = $(wildcard Assets/*.c) $(wildcard Libs/*.c)
ASM_SRCS = $(wildcard Assets/*.asm) $(wildcard Libs/*.asm)

EDITOR_C_OBJS   = $(patsubst %.c, $(OBJ_EDITOR)/%.o, $(notdir $(C_SRCS)))
EDITOR_ASM_OBJS = $(patsubst %.asm, $(OBJ_EDITOR)/%_asm.o, $(notdir $(ASM_SRCS)))
EDITOR_OBJS     = $(EDITOR_C_OBJS) $(EDITOR_ASM_OBJS)

GAME_C_OBJS   = $(patsubst %.c, $(OBJ_GAME)/%.o, $(notdir $(C_SRCS)))
GAME_ASM_OBJS = $(patsubst %.asm, $(OBJ_GAME)/%_asm.o, $(notdir $(ASM_SRCS)))
GAME_OBJS     = $(GAME_C_OBJS) $(GAME_ASM_OBJS)

vpath %.c Assets Libs
vpath %.asm Assets Libs

EDITOR = $(BUILD_ENGINE)/QEngine_Block_Code_Editor
APP    = $(BUILD_PROJECT)/$(APP_NAME)

all: prepare editor

prepare:
	@mkdir -p "$(OBJ_EDITOR)" "$(OBJ_GAME)" "$(BUILD_ENGINE)" "$(BUILD_PROJECT)" "$(FILES_PROJECT)"

editor: prepare $(EDITOR)

$(EDITOR): $(EDITOR_OBJS)
	@$(CC) -o $@ $(EDITOR_OBJS) $(LDFLAGS)

$(OBJ_EDITOR)/%.o: %.c
	@echo "Compilation C (Editor) $<..."
	@$(CC) $(CFLAGS) -DIS_EDITOR=1 -c $< -o $@

$(OBJ_EDITOR)/%_asm.o: %.asm
	@echo "Assembling ASM $<..."
	@$(ASM) $(ASMFLAGS) $< -o $@

build: prepare $(APP)

$(APP): $(GAME_OBJS)
	@$(CC) -o $@ $(GAME_OBJS) $(LDFLAGS)

$(OBJ_GAME)/%.o: %.c
	@echo "Compilation C (Game) $<..."
	@$(CC) $(CFLAGS) -DIS_EDITOR=0 -c $< -o $@

$(OBJ_GAME)/%_asm.o: %.asm
	@echo "Assembling ASM $<..."
	@$(ASM) $(ASMFLAGS) $< -o $@

run: editor
	@./$(EDITOR)

play: build
	@rsync -a --exclude={$(IGNORE_FILES)} $(FILES)/ $(FILES_PROJECT)/
	@cd $(BUILD_PROJECT) && ./$(APP_NAME)

clean:
	@rm -rf .obj $(BUILD)
	@echo "Cleaned!"

.PHONY: all prepare build editor run play clean
