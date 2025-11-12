import os
import subprocess
import glob
import tempfile
import shutil
from pathlib import Path

def compile_cpp_files():
    """Compile all .cpp files in examples directory with O3 optimization as static binaries"""
    compiled_binaries = []
    examples_dir = "examples"

    if not os.path.exists(examples_dir):
        print(f"Error: {examples_dir} directory not found!")
        return []

    cpp_files = glob.glob(os.path.join(examples_dir, "*.cpp"))

    for cpp_file in cpp_files:
        binary_name = os.path.splitext(os.path.basename(cpp_file))[0]

        try:
            # Compile with O3 optimization and static linking
            cmd = [
                "g++", "-std=c++23", "-O3", "-static", cpp_file, "-o", binary_name
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
    tests_dir = "testing/inputs"
    results = {}

    if not os.path.exists(tests_dir):
        print(f"Error: {tests_dir} directory not found!")
        return results

    test_files = glob.glob(os.path.join(tests_dir, "*"))

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

                # Step 3: Run 'test' command with the final file
                with open(final_filename, 'r') as ff:
                    result2 = subprocess.run(
                        ["./build/test"],
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

        # Also return the results array for further processing
        return results

    finally:
        # Cleanup: remove temporary directory and compiled binaries
        print(f"\nCleaning up temporary files...")

        # Remove temporary directory
        if os.path.exists(temp_dir):
            shutil.rmtree(temp_dir)
            print(f"✓ Removed temporary directory: {temp_dir}")

        # Remove compiled binaries
        cleanup_binaries(compiled_binaries)

if __name__ == "__main__":
    results = main()
