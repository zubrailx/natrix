rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MKDIR     := $(MKDIR)
CC        := $(CC)
FLAGS     := $(FLAGS)
LIBS      := $(LIBS)
INCS      := $(INCS)
BUILD_DIR := $(BUILD_DIR)
AR        := $(AR)

SRCS := $(call rwildcard,$(CURDIR),*.c)
HDRS := $(call rwildcard,$(CURDIR), *.h)
OBJS := $(patsubst $(CURDIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

DIRS := $(sort $(dir $(OBJS)))
$(sort $(DIRS)):
	$(MKDIR) $(@:dir/%=%)

.PRECIOUS: $(SRCS) $(HDRS) $(OBJS) $(BUILD_DIR)/libx86_64_core.a


$(BUILD_DIR)/%.o: %.c $(HDRS) | $(DIRS)
	$(CC) $(FLAGS) -c $< -o $@ $(INCS)

$(BUILD_DIR)/libx86_64_core.a: $(OBJS) $(LIBS)
	$(AR) rcs $@ $^

build/%:
	@$(MAKE) --no-print-directory $(patsubst build/%,$(BUILD_DIR)/%,$@)
