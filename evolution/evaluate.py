# scoring/run_code_list.py

import os
import tempfile
from pathlib import Path
import shutil
import json

# Import the existing functions and config
import sys
sys.path.append('../')
from scoring.score import compile_cpp_files, run_tests, cleanup_binaries, save_results
import config

def evaluate_cpp_code_list(code_list):
    """
    Given a list of C++ code strings, save them as .cpp files in EXAMPLES_DIR,
    compile them, run tests, and return the results.
    """
    # Ensure EXAMPLES_DIR exists
    os.makedirs(config.EXAMPLES_DIR, exist_ok=True)

    # Keep track of created files for cleanup
    created_files = []

    # Save code strings as .cpp files
    for idx, code in enumerate(code_list):
        filename = f"example_{idx + 1}.cpp"
        filepath = os.path.join(config.EXAMPLES_DIR, filename)
        with open(filepath, 'w') as f:
            f.write(code)
        created_files.append(filepath)
        print(f"✓ Saved code to {filepath}")

    # Create temporary directory for test outputs
    temp_dir = tempfile.mkdtemp(prefix="test_results_")
    print(f"Created temporary directory: {temp_dir}")

    try:
        # Compile the C++ files
        compiled_binaries = compile_cpp_files()
        if not compiled_binaries:
            print("No binaries compiled. Exiting.")
            return {}

        # Run tests
        results = run_tests(compiled_binaries, temp_dir)

        # Optionally save results
        save_results(results)

        return results

    finally:
        # Cleanup temporary files and optionally binaries
        if config.CLEANUP_AFTER:
            if os.path.exists(temp_dir):
                shutil.rmtree(temp_dir)
                print(f"✓ Removed temporary directory: {temp_dir}")
            cleanup_binaries(compiled_binaries)

        # Optional: clean up created example files (you can comment this if you want to keep them)
        for file in created_files:
            if os.path.exists(file):
                os.remove(file)
                print(f"✓ Removed temporary example file: {file}")


# Example usage
if __name__ == "__main__":
    sample_codes = [
        '#include <iostream>\nint main() { std::cout << "Hello 1"; return 0; }',
        '#include <iostream>\nint main() { std::cout << "Hello 2"; return 0; }'
    ]

    results = evaluate_cpp_code_list(sample_codes)
    print("\nResults:")
    print(json.dumps(results, indent=2))

