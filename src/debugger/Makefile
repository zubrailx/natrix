rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MKDIR       := $(MKDIR)
CP          := $(CP)
CC          := $(CC)
FLAGS       := $(FLAGS)
LIBS        := $(LIBS)
INCS        := $(INCS)
AR          := $(AR)
BUILD_DIR   := $(BUILD_DIR)


# library files
SRCS := $(filter-out $(wildcard $(CURDIR)/*.c),$(call rwildcard,$(CURDIR),*.c))
HDRS := $(filter-out $(wildcard $(CURDIR)/*.h),$(call rwildcard,$(CURDIR),*.h))
OBJS := $(patsubst $(CURDIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

DIRS := $(sort $(dir $(OBJS)) $(BUILD_DIR))
$(sort $(DIRS)):
	$(MKDIR) $(@:dir/%=%)

.PRECIOUS: $(SRCS) $(HDRS) $(OBJS) $(BUILD_DIR)/libdebugger.a


# object files
$(BUILD_DIR)/%.o: %.c $(HDRS) | $(DIRS)
	$(CC) $(FLAGS) -c $< -o $@ $(INCS)

# libraries
$(BUILD_DIR)/libdebugger.a: $(OBJS) | $(DIRS)
	$(AR) rcs $@ $^

# targets
$(BUILD_DIR)/%: $(CURDIR)/%.c $(OBJS) $(LIBS) | $(DIRS)
	$(CC) $(FLAGS) -o $@ $^ $(INCS)


build/%:
	@$(MAKE) --no-print-directory $(patsubst build/%,$(BUILD_DIR)/%,$@)
