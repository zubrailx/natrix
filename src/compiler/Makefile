rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MKDIR       := $(MKDIR)
CP          := $(CP)
CC          := $(CC)
FLAGS       := $(FLAGS)
FLAGS_GEN   := -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-type-limits
LIBS        := $(LIBS)
INCS        := $(INCS)
AR          := $(AR)
ANTLR       := $(ANTLR)
GEN_DIR     := $(GEN_DIR)
BUILD_DIR   := $(BUILD_DIR)

ANTLR_DIR           := $(CURDIR)/antlr3
ANTLR_REC_DIR       := $(ANTLR_DIR)/rec
ANTLR_TREE_DIR      := $(ANTLR_DIR)/tree

ANTLR3_RECS 		:= $(wildcard $(ANTLR_REC_DIR)/*.g3)
SRCS.ANTLR3_REC := $(patsubst $(CURDIR)/%.g3,$(GEN_DIR)/%Lexer.c, $(ANTLR3_RECS)) \
							 $(patsubst $(CURDIR)/%.g3,$(GEN_DIR)/%Parser.c, $(ANTLR3_RECS))
HDRS.ANTLR3_REC := $(patsubst $(CURDIR)/%.c,$(GEN_DIR)/%.h,$(SRCS.ANTLR3_REC))
OBJS.ANTLR3_REC := $(patsubst $(GEN_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS.ANTLR3_REC))
TKNS.ANTLR3_REC := $(patsubst $(CURDIR)/%.g3,$(GEN_DIR)/%.tokens, $(ANTLR3_RECS))
DIR.ANTLR3_REC  := $(patsubst $(CURDIR)/%, $(GEN_DIR)/%, $(ANTLR_REC_DIR))

ANTLR3_TREES := $(wildcard $(ANTLR_TREE_DIR)/*.g3)
SRCS.ANTLR3_TREE := $(patsubst $(CURDIR)/%.g3,$(GEN_DIR)/%.c, $(ANTLR3_TREES))
HDRS.ANTLR3_TREE := $(patsubst $(CURDIR)/%.c,%.h,$(SRCS.ANTLR3_TREE))
OBJS.ANTLR3_TREE := $(patsubst $(GEN_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS.ANTLR3_TREE))
TKNS.ANTLR3_TREE := $(patsubst $(CURDIR)/%.g3,$(GEN_DIR)/%.tokens, $(ANTLR3_TREES))
DIR.ANTLR3_TREE  := $(patsubst $(CURDIR)/%, $(GEN_DIR)/%, $(ANTLR_TREE_DIR))

# without generated files
SRCS.NG := $(filter-out $(wildcard $(CURDIR)/*.c),$(call rwildcard,$(CURDIR),*.c))
HDRS.NG := $(filter-out $(wildcard $(CURDIR)/*.h),$(call rwildcard,$(CURDIR), *.h))
OBJS.NG := $(patsubst $(CURDIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS.NG))

# all library files
SRCS := $(SRCS.NG) $(SRCS.ANTLR3_REC) $(SRCS.ANTLR3_TREE)
HDRS := $(HDRS.NG) $(HDRS.ANTLR3_REC) $(HDRS.ANTLR3_TREE)
OBJS := $(OBJS.NG) $(OBJS.ANTLR3_REC) $(OBJS.ANTLR3_TREE)

DIRS := $(sort $(dir $(OBJS)) $(DIR.ANTLR3_REC) $(DIR.ANTLR3_TREE))
$(sort $(DIRS)):
	$(MKDIR) $(@:dir/%=%)

.PRECIOUS: $(SRCS) $(HDRS) $(OBJS) $(TKNS.ANTLR3_REC) $(TKNS.ANTLR3_TREE) $(BUILD_DIR)/libcompiler.a


# generate grammar files and objects for recognizers (parser and lexer)
$(GEN_DIR)/%Lexer.c $(GEN_DIR)/%Parser.c $(GEN_DIR)/%Lexer.h $(GEN_DIR)/%Parser.h $(GEN_DIR)/%.tokens: %.g3 | $(DIRS)
	$(ANTLR) $^ -fo $(DIR.ANTLR3_REC)

# generate grammar files and objects for trees
$(GEN_DIR)/%.c $(GEN_DIR)/%.h $(GEN_DIR)/%.tokens: %.g3 | $(DIRS) $(TKNS.ANTLR3_REC)
	$(CP) $(TKNS.ANTLR3_REC) $(DIR.ANTLR3_TREE)
	$(ANTLR) $^ -fo $(DIR.ANTLR3_TREE)

# compile generated files
$(BUILD_DIR)/%.o: $(GEN_DIR)/%.c $(HDRS) | $(DIRS)
	$(CC) $(FLAGS_GEN) $(FLAGS) -c $< -o $@ $(INCS)

# compile object files excluding generated
$(BUILD_DIR)/%.o: %.c $(HDRS) | $(DIRS)
	$(CC) $(FLAGS) -c $< -o $@ $(INCS)

$(BUILD_DIR)/libcompiler.a: $(OBJS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%: $(CURDIR)/%.c $(OBJS) $(LIBS)
	$(CC) $(FLAGS) -o $@ $^ $(INCS)


build/%:
	@$(MAKE) --no-print-directory $(patsubst build/%,$(BUILD_DIR)/%,$@)
