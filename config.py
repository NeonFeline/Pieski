# scoring/config.py

"""
Configuration for scoring system and test runner.
"""

# ────────────────────────────────
# Directory configurations
# ────────────────────────────────
# Path to C++ example files to compile
EXAMPLES_DIR = "../examples"

# Path to input test cases
TESTS_DIR = "../testing/inputs"

# Path to test binary used for verification/scoring
TEST_BINARY = "../build/test"

# Directory for temporary and intermediate outputs
OUTPUT_DIR = "temp_results"


PENALTY = 1000000000000.0   # or any numeric penalty you prefer
TEST_TIMEOUT_SECONDS = 20  # optional override
MAX_THREADS = 8            # optional override


# ────────────────────────────────
# Compiler configuration
# ────────────────────────────────
COMPILER = "g++"
COMPILER_FLAGS = [
    "-std=c++23",
    "-O3",
    "-static",
]


# ────────────────────────────────
# Test run behavior
# ────────────────────────────────
# If True, remove compiled binaries and temp directories after tests finish.
# If False, keeps everything in place for debugging.
CLEANUP_AFTER = True




# ────────────────────────────────
# Output configuration
# ────────────────────────────────
# Output format for test results:
#   "json" → only JSON
#   "txt"  → only human-readable text
#   "both" → both JSON and text
RESULTS_FORMAT = "json"

# Filename for JSON output
RESULTS_FILENAME = "test_results.json"



# ────────────────────────────────
# Evolution configuration
# ────────────────────────────────
# Population size for evolutionary algorithm
POPULATION_SIZE = 10
GENERATIONS = 25
EVOLUTION_OUTPUT_DIR = "evolution_results"
MUTATION_CHANCE = 0.25  # Probability of mutation per individual