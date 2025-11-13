# scoring/run_tests.py

import os
import subprocess
import glob
import shutil
import json
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed, wait, FIRST_COMPLETED
import tempfile
import signal

import sys
sys.path.append('../')

# Import configuration
import config

# Fallback defaults (if not provided in config)
DEFAULT_TIMEOUT = getattr(config, "TEST_TIMEOUT_SECONDS", 20)
DEFAULT_COMPILE_TIMEOUT = getattr(config, "COMPILE_TIMEOUT_SECONDS", 30)
DEFAULT_MAX_THREADS = getattr(config, "MAX_THREADS", min(8, os.cpu_count() or 1))
PENALTY = getattr(config, "PENALTY", -1)  # value to record when a test fails/timeouts/etc.


def compile_single_cpp(cpp_file, compile_timeout=DEFAULT_COMPILE_TIMEOUT):
    """Compile a single C++ file. Returns binary name on success, None on failure."""
    binary_name = os.path.splitext(os.path.basename(cpp_file))[0]
    try:
        cmd = [
            config.COMPILER,
            *config.COMPILER_FLAGS,
            cpp_file,
            "-o",
            binary_name
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=compile_timeout)
        if result.returncode == 0:
            print(f"✓ Compiled {cpp_file} -> {binary_name}")
            return binary_name
        else:
            print(f"✗ Failed to compile {cpp_file}: {result.stderr.strip()}")
            return None
    except subprocess.TimeoutExpired:
        print(f"✗ Compilation timed out for {cpp_file}")
        return None
    except Exception as e:
        print(f"✗ Error compiling {cpp_file}: {e}")
        return None


def compile_cpp_files(max_workers=DEFAULT_MAX_THREADS):
    """Compile all .cpp files in EXAMPLES_DIR concurrently and return compiled binaries list."""
    compiled_binaries = []

    if not os.path.exists(config.EXAMPLES_DIR):
        print(f"Error: {config.EXAMPLES_DIR} directory not found!")
        return []

    cpp_files = glob.glob(os.path.join(config.EXAMPLES_DIR, "*.cpp"))
    if not cpp_files:
        print("No .cpp files found.")
        return []

    print(f"Compiling {len(cpp_files)} files using up to {max_workers} threads...")

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = {executor.submit(compile_single_cpp, cpp): cpp for cpp in cpp_files}
        for future in as_completed(futures):
            try:
                binary = future.result()
            except Exception as e:
                print(f"✗ Exception during compilation: {e}")
                binary = None
            if binary:
                compiled_binaries.append(binary)

    return compiled_binaries


def run_single_test(binary, test_file, temp_dir, timeout=DEFAULT_TIMEOUT):
    """
    Run a single binary against a single test file.
    Returns (test_name, result_or_penalty).
    """
    test_name = os.path.basename(test_file)
    try:
        # Step 1: Run the student's binary with test input
        with open(test_file, 'r') as tf:
            try:
                result1 = subprocess.run(
                    [os.path.join(".", binary)],
                    stdin=tf,
                    capture_output=True,
                    text=True,
                    timeout=timeout
                )
            except subprocess.TimeoutExpired:
                print(f"  ✗ {binary} timed out with test {test_name}")
                return test_name, PENALTY

        if result1.returncode != 0:
            # Binary crashed or returned non-zero — consider as failure/penalty
            print(f"  ✗ {binary} failed (return code {result1.returncode}) with test {test_name}: {result1.stderr.strip()}")
            return test_name, PENALTY

        # Save intermediate result (student output)
        result1_filename = os.path.join(temp_dir, f"{binary}_{test_name}_result.txt")
        with open(result1_filename, 'w') as f:
            f.write(result1.stdout)

        # Step 2: Combine test input and student output for the test evaluator
        final_filename = os.path.join(temp_dir, f"{binary}_{test_name}_final.txt")
        with open(test_file, 'r') as tf, open(final_filename, 'w') as ff:
            ff.write(tf.read() + "\n" + result1.stdout)

        # Step 3: Run the test evaluator/test binary
        with open(final_filename, 'r') as ff:
            try:
                result2 = subprocess.run(
                    [config.TEST_BINARY],
                    stdin=ff,
                    capture_output=True,
                    text=True,
                    timeout=timeout
                )
            except subprocess.TimeoutExpired:
                print(f"  ✗ {config.TEST_BINARY} timed out for {binary}_{test_name}")
                return test_name, PENALTY

        if result2.returncode != 0:
            # Evaluator failed — treat as penalty
            print(f"  ✗ test command failed for {binary}_{test_name}: {result2.stderr.strip()}")
            return test_name, PENALTY

        # Success: try to parse numeric result, else keep string
        out = result2.stdout.strip()
        try:
            test_result = float(out)
        except ValueError:
            test_result = out

        print(f"  ✓ {binary}_{test_name}: {test_result}")
        return test_name, test_result

    except Exception as e:
        # Unexpected exception during test handling — record penalty
        print(f"  ✗ Error processing {binary} with {test_name}: {e}")
        return test_name, PENALTY


def run_tests(compiled_binaries, temp_dir, max_workers=DEFAULT_MAX_THREADS):
    """
    Run all tests for all compiled binaries concurrently.
    Returns dict: { binary: { test_name: result_or_penalty, ... }, ... }
    """
    results = {}

    if not os.path.exists(config.TESTS_DIR):
        print(f"Error: {config.TESTS_DIR} directory not found!")
        return results

    test_files = glob.glob(os.path.join(config.TESTS_DIR, "*"))
    if not test_files:
        print("No test files found.")
        return results

    # Ensure output directory exists
    os.makedirs(temp_dir, exist_ok=True)

    print(f"Running tests using up to {max_workers} threads... (timeout {DEFAULT_TIMEOUT}s, penalty={PENALTY})")

    # Prepare results dict
    for binary in compiled_binaries:
        results[binary] = {}

    # Submit all (binary, test) tasks
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = {}
        for binary in compiled_binaries:
            for test_file in test_files:
                f = executor.submit(run_single_test, binary, test_file, temp_dir, DEFAULT_TIMEOUT)
                futures[f] = (binary, test_file)

        try:
            for future in as_completed(futures):
                binary, test_file = futures[future]
                test_name = os.path.basename(test_file)
                try:
                    tname, result = future.result()
                except Exception as e:
                    # If the future raised, record penalty for that test
                    print(f"  ✗ Exception for {binary} with {test_name}: {e}")
                    tname, result = test_name, PENALTY
                results[binary][tname] = result

        except KeyboardInterrupt:
            # If user hits Ctrl+C, cancel pending futures and assign penalty to them
            print("\nInterrupted by user. Cancelling pending tests...")
            # Cancel and mark remaining
            for fut, (binary, test_file) in futures.items():
                if not fut.done():
                    fut.cancel()
                    tname = os.path.basename(test_file)
                    # If not yet set, assign penalty
                    if tname not in results[binary]:
                        results[binary][tname] = PENALTY
            print("All pending tests marked with penalty and cancelled.")
    return results


def cleanup_binaries(compiled_binaries):
    """Remove compiled binaries from CWD."""
    for binary in compiled_binaries:
        path = os.path.join(".", binary)
        if os.path.exists(path):
            try:
                os.remove(path)
                print(f"✓ Removed {binary}")
            except Exception as e:
                print(f"✗ Failed to remove {binary}: {e}")


def save_results_to_json(results, filename="test_results.json"):
    """Save results to a JSON file (makes non-serializable items strings)."""
    json_serializable_results = {}
    for binary, test_results in results.items():
        json_serializable_results[binary] = {}
        for test_name, result in test_results.items():
            if result is None:
                json_serializable_results[binary][test_name] = PENALTY
            elif isinstance(result, (int, float, str)) or result is None:
                json_serializable_results[binary][test_name] = result
            else:
                json_serializable_results[binary][test_name] = str(result)
    with open(filename, 'w') as f:
        json.dump(json_serializable_results, f, indent=2, ensure_ascii=False)


def save_results_to_text(results, filename="final_results.txt"):
    with open(filename, 'w') as f:
        f.write("FINAL TEST RESULTS\n")
        f.write("=" * 50 + "\n")
        for binary, test_results in results.items():
            f.write(f"\n{binary}:\n")
            for test_name, result in test_results.items():
                f.write(f"  {test_name}: {result}\n")


def save_results(results):
    """Save results according to config.RESULTS_FORMAT."""
    if config.RESULTS_FORMAT == "json":
        save_results_to_json(results, getattr(config, "RESULTS_FILENAME", "test_results.json"))
        print(f"✓ Results saved to {getattr(config, 'RESULTS_FILENAME', 'test_results.json')}")
    elif config.RESULTS_FORMAT == "txt":
        save_results_to_text(results)
        print("✓ Results saved to final_results.txt")
    elif config.RESULTS_FORMAT == "both":
        save_results_to_json(results, getattr(config, "RESULTS_FILENAME", "test_results.json"))
        save_results_to_text(results)
        print(f"✓ Results saved to {getattr(config, 'RESULTS_FILENAME', 'test_results.json')} and final_results.txt")
    else:
        # default to json
        save_results_to_json(results, getattr(config, "RESULTS_FILENAME", "test_results.json"))
        print(f"✓ Results saved to {getattr(config, 'RESULTS_FILENAME', 'test_results.json')}")


def main():
    # Use OUTPUT_DIR from config to store intermediate files; create if missing
    temp_dir = getattr(config, "OUTPUT_DIR", "temp_results")
    os.makedirs(temp_dir, exist_ok=True)
    print(f"Using output directory: {temp_dir}")

    compiled_binaries = []
    try:
        print("Compiling C++ files...")
        compiled_binaries = compile_cpp_files()

        if not compiled_binaries:
            print("No binaries compiled. Exiting.")
            return {}

        print(f"\nCompiled {len(compiled_binaries)} binaries: {compiled_binaries}\n")
        print("Running tests...")
        results = run_tests(compiled_binaries, temp_dir)

        print("\n" + "=" * 50)
        print("FINAL RESULTS:")
        print("=" * 50)
        for binary, test_results in results.items():
            print(f"\n{binary}:")
            for test_name, result in test_results.items():
                print(f"  {test_name}: {result}")

        save_results(results)
        return results

    finally:
        if getattr(config, "CLEANUP_AFTER", False):
            print("\nCleaning up temporary files and binaries...")
            if os.path.exists(temp_dir):
                try:
                    shutil.rmtree(temp_dir)
                    print(f"✓ Removed output directory: {temp_dir}")
                except Exception as e:
                    print(f"✗ Failed to remove output directory {temp_dir}: {e}")
            cleanup_binaries(compiled_binaries)
        else:
            print(f"\nTemporary files preserved in: {temp_dir}")


if __name__ == "__main__":
    main()

