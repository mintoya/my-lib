CC = clang
DIR = build
# CFLAGS = -pthread -Wall
CFLAGS = -w -g -fsanitize=address
OBJECTS = main.c my-list/my-list.c umap/umap.c umap-printer/printer.c string-List/stringList.c
make: $(DIR) $(OBJECTS)
	$(CC) -o $(DIR)/a $(OBJECTS) $(CFLAGS)

$(DIR):
	mkdir -p $(DIR)

clean:
	rm -rf $(DIR)
run: make
	./build/a
test: make
	$(CC) -o $(DIR)/a $(OBJECTS) $(CFLAGS) -g
	gdb ./$(DIR)/a
profile: make
	@echo "perf report to view"
	@sleep 2
	perf record ./$(DIR)/a
a: $(OBJECTS)
	$(CC) -o $(DIR)/a $(OBJECTS) $(CFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)
my-list.o: my-list/my-list.c
	$(CC) -c my-list/my-list.c $(CFLAGS)
umap.o: umap/umap.c
	$(CC) -c umap/umap.c $(CFLAGS)
printer.o: umap-printer/printer.c
	$(CC) -c umap-printer/printer.c $(CFLAGS)
string-List.o: string-List/stringList.c
	$(CC) -c string-List/stringList.c $(CFLAGS)
