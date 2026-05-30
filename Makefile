CC = gcc
GLSLC = glslc

CFLAGS = -Wall -Wextra -O2 -I. -Iinclude -Ilib
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADERS_DIR = shaders
BIN = bin
BUILD = build
FILES = Files
FILESB = $(BUILD)/Files

SRCS = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJS = $(patsubst %.c, $(BIN)/%.o, $(notdir $(SRCS)))

vpath %.c src lib

APP = $(BUILD)/qengine

VERT_SRC = $(SHADERS_DIR)/shader.vert
FRAG_SRC = $(SHADERS_DIR)/shader.frag
SHADERS_SPV = $(FILESB)/vert.spv $(FILESB)/frag.spv

all: prepare $(SHADERS_SPV) $(APP)

prepare:
	@mkdir -p $(BIN) $(FILES) $(BUILD) $(FILESB)

$(FILESB)/%.spv: $(SHADERS_DIR)/shader.%
	@echo "Shader compilation $*..."
	@$(GLSLC) $< -o $@

$(BIN)/%.o: %.c
	@echo "Compilation $<..."
	@$(CC) $(CFLAGS) -c $< -o $@
$(APP): $(OBJS)
	@echo "Linking app $(APP)..."
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

run: all
	@echo "--- Starting $(APP) ---"
	@cp -r $(FILES)/ $(BUILD)/ && cd $(BUILD) && ./qengine

clean:
	@echo "Cleaning..."
	@rm -rf $(BIN) $(BUILD)

.PHONY: all prepare run clean
