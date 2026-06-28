CC = gcc
GLSLC = glslc

CFLAGS = -Wall -Wextra -O2 -I. -Iinclude -Ilib
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SHADERS_DIR = Shaders
OBJ = .obj
BUILD = Build
FILES = Assets
FILESB = $(BUILD)/$(FILES)

SRCS = $(wildcard Assets/*.c) $(wildcard Libs/*.c) $(wildcard Libs/Dev/*.c)
OBJS = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(SRCS)))

vpath %.c Assets Libs Libs/Dev

APP_NAME = QEngineApp
APP = $(BUILD)/$(APP_NAME)

VERT_SRC = $(SHADERS_DIR)/shader.vert
FRAG_SRC = $(SHADERS_DIR)/shader.frag
SHADERS_SPV = $(FILESB)/vert.spv $(FILESB)/frag.spv

all: prepare
	@$(MAKE) $(SHADERS_SPV)
	@$(MAKE) $(APP)

prepare:
	@mkdir -p $(OBJ) $(BUILD) $(FILES) $(FILESB)

$(FILESB)/%.spv: $(SHADERS_DIR)/shader.%
	@echo "Shader compilation $*..."
	@$(GLSLC) $< -o $@

$(OBJ)/%.o: %.c
	@echo "Compilation $<..."
	@$(CC) $(CFLAGS) -c $< -o $@
$(APP): $(OBJS)
	@echo "Linking app $(APP)..."
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

run: all
	@echo "--- Starting $(APP) ---"
	@cp -r $(FILES)/ $(BUILD)/
	@cd $(BUILD) && ./$(APP_NAME)

clean:
	@echo "Cleaning..."
	@rm -rf $(OBJ) $(BUILD)
	@echo "Cleaned!"

.PHONY: all prepare run clean
