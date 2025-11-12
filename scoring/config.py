# scoring/config.py

# Directory configurations
EXAMPLES_DIR = "../examples"
TESTS_DIR = "../testing/inputs"
TEST_BINARY = "../build/test"
OUTPUT_DIR = "temp_results"

# Compiler configuration
COMPILER = "g++"
COMPILER_FLAGS = ["-std=c++23", "-O3", "-static"]

# Test configuration
CLEANUP_AFTER = False  # Set to False if you want to keep temporary files

# Output configuration
RESULTS_FORMAT = "json"  # Options: "json", "txt", "both"
RESULTS_FILENAME = "test_results.json"
