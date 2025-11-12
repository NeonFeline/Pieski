# scoring/run_tests.py

import os
import subprocess
import glob
import tempfile
import shutil
import json
from pathlib import Path

# Import configuration
import config

def compile_cpp_files():
    """Compile all .cpp files in examples directory with optimization as static binaries"""
    compiled_binaries = []

    if not os.path.exists(config.EXAMPLES_DIR):
        print(f"Error: {config.EXAMPLES_DIR} directory not found!")
        return []

    cpp_files = glob.glob(os.path.join(config.EXAMPLES_DIR, "*.cpp"))

    for cpp_file in cpp_files:
        binary_name = os.path.splitext(os.path.basename(cpp_file))[0]

        try:
            # Compile with specified optimization and static linking
            cmd = [
                config.COMPILER,
                *config.COMPILER_FLAGS,
                cpp_file,
                "-o",
                binary_name
            ]

            result = subprocess.run(cmd, capture_output=True, text=True)

            if result.returncode == 0:
                print(f"✓ Compiled {cpp_file} -> {binary_name}")
                compiled_binaries.append(binary_name)
            else:
                print(f"✗ Failed to compile {cpp_file}: {result.stderr}")

        except Exception as e:
            print(f"✗ Error compiling {cpp_file}: {e}")

    return compiled_binaries

def run_tests(compiled_binaries, temp_dir):
    """Run tests for all compiled binaries and capture results"""
    results = {}

    if not os.path.exists(config.TESTS_DIR):
        print(f"Error: {config.TESTS_DIR} directory not found!")
        return results

    test_files = glob.glob(os.path.join(config.TESTS_DIR, "*"))

    for binary in compiled_binaries:
        results[binary] = {}

        for test_file in test_files:
            test_name = os.path.basename(test_file)
            print(f"Running {binary} with test {test_name}...")

            try:
                # Step 1: Run binary with test input and capture output
                with open(test_file, 'r') as tf:
                    result1 = subprocess.run(
                        [f"./{binary}"],
                        stdin=tf,
                        capture_output=True,
                        text=True
                    )

                if result1.returncode != 0:
                    print(f"  ✗ {binary} failed with test {test_name}: {result1.stderr}")
                    continue

                # Save first result to temp directory
                result1_filename = os.path.join(temp_dir, f"{binary}_{test_name}_result.txt")
                with open(result1_filename, 'w') as f:
                    f.write(result1.stdout)

                # Step 2: Combine test file with result (in temp directory)
                final_filename = os.path.join(temp_dir, f"{binary}_{test_name}_final.txt")
                with open(test_file, 'r') as tf, open(final_filename, 'w') as ff:
                    test_content = tf.read()
                    ff.write(test_content + "\n" + result1.stdout)

                # Step 3: Run test binary with the final file
                with open(final_filename, 'r') as ff:
                    result2 = subprocess.run(
                        [config.TEST_BINARY],
                        stdin=ff,
                        capture_output=True,
                        text=True
                    )

                if result2.returncode == 0:
                    # Try to parse the result as a number
                    try:
                        test_result = float(result2.stdout.strip())
                    except ValueError:
                        test_result = result2.stdout.strip()

                    results[binary][test_name] = test_result
                    print(f"  ✓ {binary}_{test_name}: {test_result}")
                else:
                    print(f"  ✗ test command failed for {binary}_{test_name}: {result2.stderr}")
                    results[binary][test_name] = None

            except Exception as e:
                print(f"  ✗ Error processing {binary} with {test_name}: {e}")
                results[binary][test_name] = None

    return results

def cleanup_binaries(compiled_binaries):
    """Clean up compiled binaries"""
    for binary in compiled_binaries:
        if os.path.exists(binary):
            os.remove(binary)
            print(f"✓ Removed {binary}")

def save_results_to_json(results, filename="test_results.json"):
    """Save results to a JSON file"""
    # Convert any non-serializable values to strings for JSON compatibility
    json_serializable_results = {}
    for binary, test_results in results.items():
        json_serializable_results[binary] = {}
        for test_name, result in test_results.items():
            # Convert non-serializable types to strings
            if result is None:
                json_serializable_results[binary][test_name] = None
            elif isinstance(result, (int, float, str)):
                json_serializable_results[binary][test_name] = result
            else:
                json_serializable_results[binary][test_name] = str(result)

    with open(filename, 'w') as f:
        json.dump(json_serializable_results, f, indent=2, ensure_ascii=False)

def save_results_to_text(results, filename="final_results.txt"):
    """Save results to a text file for human readability"""
    with open(filename, 'w') as f:
        f.write("FINAL TEST RESULTS\n")
        f.write("=" * 50 + "\n")

        for binary, test_results in results.items():
            f.write(f"\n{binary}:\n")
            for test_name, result in test_results.items():
                f.write(f"  {test_name}: {result}\n")

def save_results(results):
    """Save results in the specified format(s)"""
    if config.RESULTS_FORMAT == "json":
        save_results_to_json(results, config.RESULTS_FILENAME)
        print(f"✓ Results saved to {config.RESULTS_FILENAME}")
    elif config.RESULTS_FORMAT == "txt":
        save_results_to_text(results)
        print(f"✓ Results saved to final_results.txt")
    elif config.RESULTS_FORMAT == "both":
        save_results_to_json(results, config.RESULTS_FILENAME)
        save_results_to_text(results)
        print(f"✓ Results saved to {config.RESULTS_FILENAME} and final_results.txt")

def main():
    # Create temporary directory
    temp_dir = tempfile.mkdtemp(prefix="test_results_")
    print(f"Created temporary directory: {temp_dir}")

    try:
        print("Compiling C++ files...")
        compiled_binaries = compile_cpp_files()

        if not compiled_binaries:
            print("No binaries compiled. Exiting.")
            return

        print(f"\nCompiled {len(compiled_binaries)} binaries: {compiled_binaries}")

        print("\nRunning tests...")
        results = run_tests(compiled_binaries, temp_dir)

        print("\n" + "="*50)
        print("FINAL RESULTS:")
        print("="*50)

        # Display results in a structured format
        for binary, test_results in results.items():
            print(f"\n{binary}:")
            for test_name, result in test_results.items():
                print(f"  {test_name}: {result}")

        # Save results in specified format
        save_results(results)

        return results

    finally:
        # Cleanup based on config setting
        if config.CLEANUP_AFTER:
            print(f"\nCleaning up temporary files...")

            # Remove temporary directory
            if os.path.exists(temp_dir):
                shutil.rmtree(temp_dir)
                print(f"✓ Removed temporary directory: {temp_dir}")

            # Remove compiled binaries
            cleanup_binaries(compiled_binaries)
        else:
            print(f"\nTemporary files preserved in: {temp_dir}")

if __name__ == "__main__":
    results = main()
