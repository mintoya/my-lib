CC = zig cc
DIR = build
CFLAGS = -w -O3
# CFLAGS = -fsanitize=address -g -O0 -w

OBJECTS = ./examples/omapTest.c

UNAME_S := $(shell uname -s 2>/dev/null)

EXECUTABLE = a.exe

# TODO
# gcc -fdump-tree-original printexample.c
# add later
make: $(DIR) $(OBJECTS)
	$(CC) -o $(DIR)/$(EXECUTABLE) $(OBJECTS) $(CFLAGS)

$(DIR):
	mkdir -p $(DIR)

clean:
	rm -rf $(DIR)
run: make
	./$(DIR)/$(EXECUTABLE) 
debug: 
	gcc -o $(DIR)/$(EXECUTABLE) $(OBJECTS) -g
	gdb --tui ./$(DIR)/$(EXECUTABLE)
profile: make
	@echo "perf report to view"
	@sleep 2
	perf record ./$(DIR)/$(EXECUTABLE)
