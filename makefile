CC = g++
DIR = build
# CFLAGS = -w -O3
CFLAGS = -fsanitize=address -g -O0 -w

OBJECTS = ./examples/kmltest.c

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(UNAME_S),Darwin)    
    EXECUTABLE = a
else ifeq ($(UNAME_S),Linux) 
    EXECUTABLE = a
else ifneq ($(findstring MINGW,$(UNAME_S)),)
    EXECUTABLE = a.exe
else ifneq ($(findstring CYGWIN,$(UNAME_S)),)
    EXECUTABLE = a.exe
else ifneq ($(findstring MSYS,$(UNAME_S)),) 
    EXECUTABLE = a.exe
else                                       
    EXECUTABLE = a
endif


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
	gcc -o $(DIR)/$(EXECUTABLE) $(OBJECTS) $(CFLAGS) -g
	gdb --tui ./$(DIR)/$(EXECUTABLE)
profile: make
	@echo "perf report to view"
	@sleep 2
	perf record ./$(DIR)/$(EXECUTABLE)
