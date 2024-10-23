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


def run_c_program(input_data):
    try:
        # Run the C program and send the input_data as stdin
        process = subprocess.Popen(["./ep1"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate(input=input_data)

        if process.returncode != 0:
            print(f"Error running C program: {stderr}")
            return
        
    except Exception as e:
        print(f"An error occurred: {e}")
        return

    return stdout

def main():
    # Run tests
    for index, input_file, output_file in test_cases:
        print(f"\nRunning test case {index}...")

        # Run the program with input redirection
        with open(input_file, 'r') as infile:
            input_data = infile.read()

        output_data = run_c_program(input_data)

        # Read the expected output
        with open(output_file, 'r') as expected_file:
            expected_output = expected_file.read()

        # Compare outputs
        if output_data.strip() == expected_output.strip():
            print(f"Test case {index} passed.")
        else:
            print(f"Test case {index} failed.")
            print(f"Expected:\n{expected_output}")
            print(f"Actual:\n{output_data}")

if __name__ == "__main__":
    main()
