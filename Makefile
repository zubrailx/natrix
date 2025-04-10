rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

MKDIR          ?= mkdir -p
CP             ?= cp -f
ANTLR          ?= java -jar /usr/local/maven/m2/repository/org/antlr/antlr-complete/3.5.3/antlr-complete-3.5.3.jar
FORMAT         ?= clang-format-20 --verbose -i
CC             ?= gcc
AR             ?= ar
AS             ?= as
LD             ?= ld

FLAGS_SANITIZE ?= -fsanitize=address,undefined,leak
# -DNDEBUG
FLAGS_APPEND   ?=
FLAGS          ?= -g3 -O0 -m64 -Wall -Wextra $(FLAGS_SANITIZE) $(FLAGS_APPEND)

BUILD_DIR      ?= $(CURDIR)/build
GEN_DIR        ?= $(BUILD_DIR)/_gen
TARGET_DIR     ?= $(BUILD_DIR)
TARGET_TST_DIR ?= $(BUILD_DIR)/test

PDF.COMPILER_FLAGS ?= --ast --cfg --cfg-add-expr --cg --hir-tree --hir-symbols --hir-types --mir

ASM.AS_FLAGS       ?= --64 -g
ASM.LD_FLAGS       ?= -g -z noexecstack
ASM.COMPILER_FLAGS ?=
ASM.DL_PATH        ?= /usr/lib64/ld-linux-x86-64.so.2

COMPILE.AS_FLAGS   ?= --64 -g
COMPILE.LD_FLAGS   ?= -g -z noexecstack
COMPILE.DL_PATH    ?= /usr/lib64/ld-linux-x86-64.so.2

SRC_DIR := $(CURDIR)/src

util.SRC_DIR   := $(SRC_DIR)/util
util.BUILD_DIR := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(util.SRC_DIR))
util.FLAGS     := -g3 -O0 -m64 -Wall -Wextra

x86_64_core.SRC_DIR   := $(SRC_DIR)/x86_64_core
x86_64_core.BUILD_DIR := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(x86_64_core.SRC_DIR))
x86_64_core.LIBS      := $(util.BUILD_DIR)/libutil.a
x86_64_core.FLAGS     := -g3 -O0 -m64 -Wall -Wextra

compiler.SRC_DIR   := $(SRC_DIR)/compiler
compiler.BUILD_DIR := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(compiler.SRC_DIR))
compiler.GEN_DIR   := $(patsubst $(SRC_DIR)/%,$(GEN_DIR)/%, $(compiler.SRC_DIR))
compiler.LIBS      := $(util.BUILD_DIR)/libutil.a $(x86_64_core.BUILD_DIR)/libx86_64_core.a
compiler.FLAGS     := $(FLAGS)

x86_64_std.SRC_DIR   := $(SRC_DIR)/x86_64_std
x86_64_std.BUILD_DIR := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(x86_64_std.SRC_DIR))
x86_64_std.FLAGS     := -g3 -O0 -m64 -Wall -Wextra

debugger.SRC_DIR   := $(SRC_DIR)/debugger
debugger.BUILD_DIR := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(debugger.SRC_DIR))
debugger.LIBS      := $(x86_64_core.BUILD_DIR)/libx86_64_core.a $(util.BUILD_DIR)/libutil.a
debugger.FLAGS     := $(FLAGS)

test.SRC_DIR   := $(CURDIR)/test
test.BUILD_DIR := $(patsubst $(CURDIR)/%,$(BUILD_DIR)/%,$(test.SRC_DIR))
test.LIBS      := $(debugger.BUILD_DIR)/libdebugger.a $(compiler.BUILD_DIR)/libcompiler.a $(x86_64_core.BUILD_DIR)/libx86_64_core.a $(x86_64_std.BUILD_DIR)/libx86_64_std.a  $(util.BUILD_DIR)/libutil.a

MODULES := util compiler x86_64_core x86_64_std debugger test

.DEFAULT_GOAL := all
.PHONY := clean/build clean/out compiler debugger format help index run/asm run/pdf run/test x86_64_std test util x86_64_core
.PRECIOUS: $(util.LIBS) $(compiler.LIBS) $(x86_64_core.LIBS) $(x86_64_std.LIBS) $(debugger.LIBS) $(test.LIBS)

FORCE:


define compiler.ENV
	MKDIR="$(MKDIR)" \
	CP="$(CP)" \
	CC="$(CC)" \
	FLAGS="$(compiler.FLAGS)" \
	LIBS="$(compiler.LIBS)" \
	INCS="$(patsubst %,-I%,$(sort $(SRC_DIR) $(GEN_DIR))) -lantlr3c" \
	ANTLR="$(ANTLR)" \
	GEN_DIR="$(compiler.GEN_DIR)" \
	BUILD_DIR="$(compiler.BUILD_DIR)"
endef

define util.ENV
	MKDIR="$(MKDIR)" \
	CC="$(CC)" \
	FLAGS="$(util.FLAGS)" \
	INCS="-I$(SRC_DIR)" \
	BUILD_DIR="$(util.BUILD_DIR)" \
	AR="$(AR)"
endef

define x86_64_core.ENV
	MKDIR="$(MKDIR)" \
	CC="$(CC)" \
	FLAGS="$(x86_64_core.FLAGS)" \
	LIBS="$(x86_64_core.LIBS)" \
	INCS="-I$(SRC_DIR)" \
	BUILD_DIR="$(x86_64_core.BUILD_DIR)"
endef

define x86_64_std.ENV
	MKDIR="$(MKDIR)" \
	CC="$(CC)" \
	FLAGS="$(x86_64_std.FLAGS)" \
	INCS="-I$(SRC_DIR)" \
	BUILD_DIR="$(x86_64_std.BUILD_DIR)"
endef

define debugger.ENV
	MKDIR="$(MKDIR)" \
	CC="$(CC)" \
	FLAGS="$(debugger.FLAGS)" \
	LIBS="$(debugger.LIBS)" \
	INCS="-I$(SRC_DIR) -lbfd -lopcodes" \
	BUILD_DIR="$(debugger.BUILD_DIR)"
endef

define test.ENV
	MKDIR="$(MKDIR)" \
	CC="$(CC)" \
	FLAGS="$(FLAGS)" \
	LIBS="$(test.LIBS)" \
	INCS="-I$(SRC_DIR) -lantlr3c -lcriterion -lbfd -lopcodes" \
	BUILD_DIR="$(test.BUILD_DIR)"
endef


define make-module-rule
$(1)/%: $$($(1).LIBS)
	@$$(MAKE) --no-print-directory -C $$($(1).SRC_DIR) $$(patsubst $(1)/%,%,$$@) $$($(1).ENV)
endef

$(foreach module, $(MODULES), $(eval $(call make-module-rule,$(module))))

### Execute any target of submakefile (recursively build required libs), transitive
/%: FORCE
	@$(eval TARGET_NAME=$(firstword $(subst /, ,$(patsubst $(BUILD_DIR)/%,%,$@)))) \
	$(eval ENV_NAME=$(patsubst %,%.ENV,$(TARGET_NAME))) \
	$(foreach lib,$($(TARGET_NAME).LIBS),$(MAKE) --no-print-directory $(lib) &&) \
	$(MAKE) --no-print-directory -C $(SRC_DIR)/$(TARGET_NAME) $@ $($(ENV_NAME))

### Build compiler x86_64_core x86_64_std debugger
all: compiler x86_64_core x86_64_std debugger

### Build libutil.a
util: util/build/libutil.a

### Build compiler executable
compiler: compiler/build/main

### Build libx86_64_core.a
x86_64_core: x86_64_core/build/libx86_64_core.a

### Build libx86_64_std.a
x86_64_std: x86_64_std/build/libx86_64_std.a

### Build debugger executable
debugger: debugger/build/main

### Build tests
test: test/build

### Run all tests or specific
run/test: FORCE | test/build
	@for file in $$(find $(test.BUILD_DIR) -executable -type f); do echo "Test file: $$file" && "$$file"; done;
run/test/%: test/build/%
	@$(eval TEST_FILE=$(patsubst run/test/%,$(test.BUILD_DIR)/%,$@)) \
		echo "Test file: $(TEST_FILE)" && $(TEST_FILE)

### Run main executable and generate dot
run/pdf: run/pdf/io/suite28.txt
run/pdf/%: PDF.INPUT_FILE=$(patsubst run/pdf/%,%,$@)
run/pdf/%: PDF.ASM_FILE=$(patsubst run/pdf/%,%.asm,$@)
run/pdf/%: compiler/build/main
	./build/compiler/main -d . -o $(PDF.ASM_FILE) \
		$(PDF.COMPILER_FLAGS) \
		$(PDF.INPUT_FILE) || true
	@echo "--------------------DOT2PDF($(PDF.INPUT_FILE))--------------------"
	for file in $$(find io -type f -name "*.dot"); do dot -Tpdf "$$file" -o "$${file%.dot}.pdf" && echo "converted $$file" || true; done

### Compile file
run/asm: run/asm/io/suite28.txt
run/asm/%: ASM.INPUT_FILE=$(patsubst run/asm/%,%,$@)
run/asm/%: ASM.ASM_FILE=$(patsubst run/asm/%,%.asm,$@)
run/asm/%: ASM.OBJECT_FILE=$(patsubst run/asm/%,%.o,$@)
run/asm/%: ASM.OUTPUT_FILE=$(patsubst run/asm/%,%.out,$@)
run/asm/%: compiler/build/main
	./build/compiler/main $(ASM.COMPILER_FLAGS) \
		-o $(ASM.ASM_FILE) \
		$(ASM.INPUT_FILE) $(x86_64_std.SRC_DIR)/x86_64_std.txt
	@echo "--------------------ASSEMBLY($(ASM.ASM_FILE))--------------------"
	@cat $(ASM.ASM_FILE)
	@echo "--------------------COMPILE($(ASM.ASM_FILE))--------------------"
	@$(MAKE) --no-print-directory run/compile/$(ASM.ASM_FILE)

run/compile: run/compile/io/suite28.txt.asm
run/compile/%: COMPILE.ASM_FILE=$(patsubst run/compile/%,%,$@)
run/compile/%: COMPILE.OBJECT_FILE=$(patsubst run/compile/%,%.o,$@)
run/compile/%: COMPILE.OUTPUT_FILE=$(patsubst run/compile/%,%.out,$@)
run/compile/%: x86_64_core/build/libx86_64_core.a x86_64_std/build/libx86_64_std.a
	$(AS) $(COMPILE.AS_FLAGS) -o $(COMPILE.OBJECT_FILE) $(COMPILE.ASM_FILE)
	$(LD) $(COMPILE.LD_FLAGS) -o $(COMPILE.OUTPUT_FILE) \
		-dynamic-linker $(COMPILE.DL_PATH) \
		/usr/lib/x86_64-linux-gnu/crt1.o \
		/usr/lib/x86_64-linux-gnu/crti.o \
		-lc \
		$(COMPILE.OBJECT_FILE) \
		$(x86_64_core.BUILD_DIR)/libx86_64_core.a \
		$(x86_64_std.BUILD_DIR)/libx86_64_std.a \
		$(util.BUILD_DIR)/libutil.a \
		/usr/lib/x86_64-linux-gnu/crtn.o

### Run debugger
run/dbg: run/dbg/io/dbg_init.txt
run/dbg/%: DBG.INIT_FILE=$(patsubst run/dbg/%,%,$@)
run/dbg/%: debugger/build/main
	./build/debugger/main -i $(DBG.INIT_FILE)

### Format
format:
	@$(FORMAT) $(call rwildcard,$(SRC_DIR),*.c) $(call rwildcard,$(SRC_DIR),*.h) \
		 $(call rwildcard,$(TEST.SRC_DIR),*.c) $(call rwildcard,$(TEST.SRC_DIR),*.h)

### Index project
index:
	make clean/build
	bear          -- make -j$(nproc) util
	bear --append -- make -j$(nproc) x86_64_core
	bear --append -- make -j$(nproc) compiler
	bear --append -- make -j$(nproc) debugger
	bear --append -- make -j$(nproc) test/build

clean: clean/build

### Remove build files
clean/build:
	rm -rf $(BUILD_DIR)

### Remove io output files
clean/out:
	find io -type f -name "*.dot" -delete
	find io -type f -name "*.pdf" -delete
	find io -type f -name "*.asm" -delete
	find io -type f -name "*.o" -delete
	find io -type f -name "*.out" -delete

### Help
help:
	@printf "Targets:\n"
	@awk '/^[a-zA-Z\-_0-9%:\\]+/ { \
		helpMessage = match(lastLine, /^###(.*)/); \
		if (helpMessage) { \
			helpCommand = $$1; \
			helpMessage = substr(lastLine, RSTART + 3, RLENGTH); \
			gsub("\\\\", "", helpCommand); \
			gsub(":+$$", "", helpCommand); \
			printf "  \x1b[32;01m%-26s\x1b[0m %s\n", helpCommand, helpMessage; \
		} \
	} \
	{ lastLine = $$0 }' $(MAKEFILE_LIST) | sort -u
	@printf "Modules (prefixes of module targets):\n  \033[32;01m$(MODULES)\033[0m \n"
