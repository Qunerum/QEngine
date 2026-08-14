CC = gcc
ASM = nasm
GLSLC = glslc

CFLAGS = -Wall -Wextra -O2 -I. -Iinclude -Ilib -ffunction-sections -fdata-sections
ASMFLAGS = -f elf64
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -Wl,--gc-sections -lasound -lpthread -lm

OBJ = .obj
BUILD = Build
FILES = Assets
FILESB = $(BUILD)/$(FILES)

C_SRCS   = $(wildcard Assets/*.c) $(wildcard Libs/*.c) $(wildcard Libs/Dev/*.c)
ASM_SRCS = $(wildcard Assets/*.asm) $(wildcard Libs/*.asm) $(wildcard Libs/Dev/*.asm)

C_OBJS   = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(C_SRCS)))
ASM_OBJS = $(patsubst %.asm, $(OBJ)/%_asm.o, $(notdir $(ASM_SRCS)))

OBJS = $(C_OBJS) $(ASM_OBJS)

vpath %.c Assets Libs
vpath %.asm Assets Libs

APP_NAME = QEngineApp
APP = $(BUILD)/$(APP_NAME)

all: prepare
	@$(MAKE) $(APP)

prepare:
	@mkdir -p $(OBJ) $(BUILD) $(FILES) $(FILESB)

$(OBJ)/%.o: %.c
	@echo "Compilation C $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%_asm.o: %.asm
	@echo "Assembling ASM $<..."
	@$(ASM) $(ASMFLAGS) $< -o $@

$(APP): $(OBJS)
	@echo "Linking app $(APP)..."
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

run: all
	@echo "--- Starting $(APP) ---"
	@rsync -a --exclude={'*.c','*.h','*.asm'} $(FILES)/ $(BUILD)/
	@cd $(BUILD) && ./$(APP_NAME)

clean:
	@echo "Cleaning..."
	@rm -rf $(OBJ) $(BUILD)
	@echo "Cleaned!"

.PHONY: all prepare run clean
