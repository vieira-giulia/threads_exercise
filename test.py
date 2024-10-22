import subprocess
import os

# Define the directory containing test cases
test_cases_dir = "test_cases"

# List of test cases with input and output file names
test_cases = [
    (1, os.path.join(test_cases_dir, "input_01.txt"), os.path.join(test_cases_dir, "output_01.txt")),
    (2, os.path.join(test_cases_dir, "input_02.txt"), os.path.join(test_cases_dir, "output_02.txt")),
    (3, os.path.join(test_cases_dir, "input_03.txt"), os.path.join(test_cases_dir, "output_03.txt")),
    (4, os.path.join(test_cases_dir, "input_04.txt"), os.path.join(test_cases_dir, "output_04.txt")),
    (5, os.path.join(test_cases_dir, "input_05.txt"), os.path.join(test_cases_dir, "output_05.txt")),
    (6, os.path.join(test_cases_dir, "input_06.txt"), os.path.join(test_cases_dir, "output_06.txt")),
    (7, os.path.join(test_cases_dir, "input_07.txt"), os.path.join(test_cases_dir, "output_07.txt")),
    (8, os.path.join(test_cases_dir, "input_08.txt"), os.path.join(test_cases_dir, "output_08.txt")),
    (9, os.path.join(test_cases_dir, "input_09.txt"), os.path.join(test_cases_dir, "output_09.txt")),
    (10, os.path.join(test_cases_dir, "input_10.txt"), os.path.join(test_cases_dir, "output_10.txt")),
]

def run_command(command, input_text=None):
    """Helper function to run a command and return its output."""
    result = subprocess.run(command, input=input_text, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return result.stdout.strip(), result.stderr.strip()

def compare_outputs(expected_output, actual_output):
    """Compares the expected output with the actual output."""
    return expected_output.strip() == actual_output.strip()

def main():
    # Clean, build the project
    print("Cleaning project...")
    stdout, stderr = run_command(["make", "clean"])
    if stderr:
        print("Error during cleaning:", stderr)

    print("Building project...")
    stdout, stderr = run_command(["make"])
    if stderr:
        print("Error during build:", stderr)

    # Run tests
    for index, input_file, output_file in test_cases:
        print(f"\nRunning test case {index}...")

        # Run the program with input redirection
        with open(input_file, 'r') as infile:
            input_content = infile.read()
            stdout, stderr = run_command(["make", "run"], input_text=input_content)

        # Read the expected output
        with open(output_file, 'r') as expected_file:
            expected_output = expected_file.read()

        # Compare outputs
        if compare_outputs(expected_output, stdout):
            print(f"Test case {index} passed.")
        else:
            print(f"Test case {index} failed.")
            print(f"Expected:\n{expected_output}")
            print(f"Actual:\n{stdout}")

if __name__ == "__main__":
    main()
