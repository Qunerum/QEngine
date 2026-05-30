CC = gcc
GLSLC = glslc

CFLAGS = -Wall -Wextra -O2 -I. -Iinclude -Ilib
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADERS_DIR = shaders
BIN = bin
BUILD = build

SRCS = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJS = $(patsubst %.c, $(BIN)/%.o, $(notdir $(SRCS)))

vpath %.c src lib

APP_TARGET = $(BUILD)/qengine

VERT_SRC = $(SHADERS_DIR)/shader.vert
FRAG_SRC = $(SHADERS_DIR)/shader.frag
SHADERS_SPV = $(BIN)/vert.spv $(BIN)/frag.spv

all: prepare $(SHADERS_SPV) $(APP_TARGET)

prepare:
	@mkdir -p $(BIN)
	@mkdir -p $(BUILD)

$(BIN)/%.spv: $(SHADERS_DIR)/shader.%
	@echo "Shader compilation $*..."
	@$(GLSLC) $< -o $@

$(BIN)/%.o: %.c
	@echo "Compilation $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(APP_TARGET): $(OBJS)
	@echo "Linking app $(APP_TARGET)..."
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

run: all
	@echo "--- Starting $(APP_TARGET) ---"
	@./$(APP_TARGET)

clean:
	@echo "Cleaning..."
	@rm -rf $(BIN) $(BUILD)

.PHONY: all prepare run clean
