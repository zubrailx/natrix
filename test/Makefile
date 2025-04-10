rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MKDIR     := $(MKDIR)
CC        := $(CC)
FLAGS     := $(FLAGS)
LIBS      := $(LIBS)
INCS      := $(INCS)
BUILD_DIR := $(BUILD_DIR)

SRCS := $(call rwildcard,$(CURDIR),*.c)
HDRS := $(call rwildcard,$(CURDIR), *.h)
OBJS := $(patsubst $(CURDIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TGTS := $(patsubst %.o,%,$(OBJS))

DIRS := $(sort $(dir $(OBJS)))
$(sort $(DIRS)):
	$(MKDIR) $(@:dir/%=%)

# compile object files excluding generated
$(BUILD_DIR)/%: $(CURDIR)/%.c $(LIBS) | $(DIRS)
	$(CC) $(FLAGS) $^ -o $@ $(INCS)

build/%:
	@$(MAKE) --no-print-directory $(patsubst build/%,$(BUILD_DIR)/%,$@)

build: $(TGTS)
