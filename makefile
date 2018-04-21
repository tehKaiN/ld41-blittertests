# Windows version of VBCC requires absolute path in all .h files
# e.g. timer manager has to refer to timer.h by absolute path

# TODO: refactor so 'make' without args won't recompile whole ACE

# Multi-platform
ifdef ComSpec
	# Windows
	RM = del
	CP = copy
	SLASH = \\
	CURR_DIR=$(shell chdir)
	ECHO = @echo
	NEWLINE = @echo.
	QUIETCOPY = > NUL
else
	# Linux/Amiga
	RM = rm
	CP = cp
	SLASH = /
	CURR_DIR = $(shell pwd)
	ECHO = @echo
	NEWLINE = @echo " "
	QUIETCOPY =
endif
SL= $(strip $(SLASH))
SRC_DIR = $(CURR_DIR)$(SL)src

# Directories
TMP_DIR = build
ACE_DIR = ..$(SL)ace
ACE_INC_DIR = $(ACE_DIR)$(SL)include

# Compiler stuff
LD41_CC ?= vc

INCLUDES = -I$(SRC_DIR) -I$(ACE_DIR)/include
ifeq ($(LD41_CC), vc)
	CC_FLAGS = +kick13 -c99 $(INCLUDES) -DAMIGA
	ACE_AS = vc
	AS_FLAGS = +kick13 -c
	OBJDUMP =
else ifeq ($(LD41_CC), m68k-amigaos-gcc)
	CC_FLAGS = -std=gnu11 $(INCLUDES) -DAMIGA -noixemul -Wall -fomit-frame-pointer -O3
	ACE_AS = vasm
	AS_FLAGS = -quiet -x -m68010 -Faout
	OBJDUMP = m68k-amigaos-objdump -S -d $@ > $@.dasm
endif

# File list
LD41_MAIN_FILES = $(wildcard $(SRC_DIR)/*.c)
LD41_MAIN_OBJS = $(addprefix $(TMP_DIR)$(SL), $(notdir $(LD41_MAIN_FILES:.c=.o)))

LD41_GS_GAME_FILES = $(wildcard $(SRC_DIR)/gamestates/game/*.c)
LD41_GS_GAME_OBJS = $(addprefix $(TMP_DIR)$(SL)gsgame_, $(notdir $(LD41_GS_GAME_FILES:.c=.o)))

#LD41_GS_MENU_FILES = $(wildcard $(SRC_DIR)/gamestates/menu/*.c)
#LD41_GS_MENU_OBJS = $(addprefix $(TMP_DIR)$(SL)gsmenu_, $(notdir $(LD41_GS_MENU_FILES:.c=.o)))

LD41_FILES = $(LD41_MAIN_FILES) $(LD41_GS_GAME_FILES) $(LD41_GS_MENU_FILES)
LD41_OBJS = $(LD41_MAIN_OBJS) $(LD41_GS_GAME_OBJS) $(LD41_GS_MENU_OBJS)
ACE_OBJS = $(wildcard $(ACE_DIR)/build/*.o)

#
ace: $(ACE_OBJS)
	-make -C $(ACE_DIR) all ACE_CC=$(LD41_CC)
	$(NEWLINE)
	$(ECHO) Copying ACE objs
	$(NEWLINE)
	@$(CP) $(ACE_DIR)$(SL)build$(SL)*.o $(TMP_DIR) $(QUIETCOPY)

LD41: $(LD41_OBJS)
	$(NEWLINE)
	$(ECHO) Linking...
	@$(LD41_CC) $(CC_FLAGS) -lamiga -o $@ $^ $(ACE_OBJS)

# Main files
$(TMP_DIR)$(SL)%.o: $(SRC_DIR)/%.c
	$(ECHO) Building $<
	@$(LD41_CC) $(CC_FLAGS) -c -o $@ $<

# Game
$(TMP_DIR)$(SL)gsgame_%.o: $(SRC_DIR)/gamestates/game/%.c
	$(ECHO) Building $<
	@$(LD41_CC) $(CC_FLAGS) -c -o $@ $<

# Menu
#$(TMP_DIR)$(SL)gsmenu_%.o: $(SRC_DIR)/gamestates/menu/%.c
#	$(ECHO) Building $<
#	@$(LD41_CC) $(CC_FLAGS) -c -o $@ $<

all: clear ace LD41

clear:
	$(ECHO) "a" > $(TMP_DIR)$(SL)foo.o
	$(RM) $(TMP_DIR)$(SL)*.o
