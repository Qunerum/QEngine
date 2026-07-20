CC = gcc
GLSLC = glslc

CFLAGS = -Wall -Wextra -O2 -I. -Iinclude -Ilib
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

OBJ = .obj
BUILD = Build
FILES = Assets
FILESB = $(BUILD)/$(FILES)

SRCS = $(wildcard Assets/*.c) $(wildcard Libs/*.c)
OBJS = $(patsubst %.c, $(OBJ)/%.o, $(notdir $(SRCS)))

vpath %.c Assets Libs Libs/Dev

APP_NAME = QEngineApp
APP = $(BUILD)/$(APP_NAME)

all: prepare
	@$(MAKE) $(APP)

prepare:
	@mkdir -p $(OBJ) $(BUILD) $(FILES) $(FILESB)

$(OBJ)/%.o: %.c
	@echo "Compilation $<..."
	@$(CC) $(CFLAGS) -c $< -o $@
$(APP): $(OBJS)
	@echo "Linking app $(APP)..."
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

run: all
	@echo "--- Starting $(APP) ---"
	@rsync -av --exclude={'*.c','*.h'} $(FILES)/ $(BUILD)/
	@cd $(BUILD) && ./$(APP_NAME)

clean:
	@echo "Cleaning..."
	@rm -rf $(OBJ) $(BUILD)
	@echo "Cleaned!"

.PHONY: all prepare run clean
