.PHONY: all run test clean

BUILD_DIR = build
SOURCE_DIR = source
TEST_DIR = test

$(BUILD_DIR)/solution: $(SOURCE_DIR)/main.c
	clang -std=c99 -Wall -pedantic $(SOURCE_DIR)/*.c -o $(BUILD_DIR)/solution

$(BUILD_DIR)/solution_test: $(TEST_DIR)/main.c
	clang -std=c99 -Wall -pedantic $(TEST_DIR)/*.c -o $(BUILD_DIR)/solution_test

all: $(SOURCE_DIR)/main.c $(TEST_DIR)/main.c
	clang -std=c99 -Wall -pedantic $(SOURCE_DIR)/*.c -o $(BUILD_DIR)/solution
	clang -std=c99 -Wall -pedantic $(TEST_DIR)/*.c -o $(BUILD_DIR)/solution_test

run: $(BUILD_DIR)/solution
	@./$(BUILD_DIR)/solution

test: $(BUILD_DIR)/solution_test
	@./$(BUILD_DIR)/solution_test

clean:
	@rm ./$(BUILD_DIR)/*