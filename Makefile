.PHONY: all run build test clean tar res

BUILD_DIR = build
SOURCE_DIR = source
TEST_DIR = test


# clang -std=c99 -Wall -pedantic ./*.c -o ../solution

$(BUILD_DIR)/solution: $(SOURCE_DIR)/main.c $(SOURCE_DIR)/*.c $(SOURCE_DIR)/*.h
	@clang -std=c99 -Wall -pedantic $(SOURCE_DIR)/*.c -o $(BUILD_DIR)/solution -L./$(SOURCE_DIR) -lruntime

$(BUILD_DIR)/solution_test: $(TEST_DIR)/main.c
	@clang -std=c99 -Wall -pedantic $(TEST_DIR)/*.c -o $(BUILD_DIR)/solution_test -L./$(SOURCE_DIR) -lruntime

all: $(SOURCE_DIR)/main.c $(TEST_DIR)/main.c $(SOURCE_DIR)/*.c $(SOURCE_DIR)/*.h
	@clang -std=c99 -Wall -pedantic $(SOURCE_DIR)/*.c -o $(BUILD_DIR)/solution -L./$(SOURCE_DIR) -lruntime
	@clang -std=c99 -Wall -pedantic $(TEST_DIR)/*.c -o $(BUILD_DIR)/solution_test -L./$(SOURCE_DIR) -lruntime

run: $(BUILD_DIR)/solution
	printenv | grep LD_LIBRARY_PATH
	@./$(BUILD_DIR)/solution -p 3 10 20 30 40 50 60 70 80 90

run_no_opt: $(BUILD_DIR)/solution
	@./$(BUILD_DIR)/solution

build: $(BUILD_DIR)/solution

test: $(BUILD_DIR)/solution_test
	@./$(BUILD_DIR)/solution_test -p 3 11 22 33

clean:
	@rm -r ./$(BUILD_DIR)/*

tar:$(BUILD_DIR)/solution
	mkdir $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/bank_worker.* $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/communicator.* $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/logger.* $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/message_builder.* $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/lamport_ipc.* $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/ipc.c $(BUILD_DIR)/pa4
	cp $(SOURCE_DIR)/main.c $(BUILD_DIR)/pa4
	tar -czpf $(BUILD_DIR)/pa3.tar.gz -C $(BUILD_DIR) pa4
	explorer.exe $(BUILD_DIR)

res:
	rm -r ./res/*
	cp /mnt/c/Users/kimda/Downloads/pa3_results.tar.gz.gpg res/
	gpg -d res/pa4_results.tar.gz.gpg > res/pa4_results.tar.gz
	explorer.exe res

