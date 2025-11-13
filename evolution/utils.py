from openai import OpenAI

# Connect to your local vLLM server
client = OpenAI(
    base_url="http://localhost:8000/v1",
    api_key="EMPTY"  # vLLM ignores this
)

MODEL = "openai/gpt-oss-120b"

def load_prompt(file_path):
    with open(file_path, "r") as f:
        return f.read()

PROBLEM_STATEMENT = load_prompt("prompts/problem_statement.txt")
MUTATE_PROMPT = load_prompt("prompts/mutate_prompt.txt")
REPRODUCE_PROMPT = load_prompt("prompts/reproduce_prompt.txt")

def call_model(messages):

    """Helper function to call the vLLM model with a given conversation."""
    response = client.chat.completions.create(
        model=MODEL,
        messages=messages,
        temperature=0.7,
        extra_body={"reasoning": {"enabled": True}}
    )
    return response.choices[0].message.content

def extract_code(text, lang="cpp"):
    """
    Extracts code snippets from the model's response.
    """
    import re
    pattern = rf"```{lang}\n(.*?)```"
    matches = re.findall(pattern, text, re.DOTALL)
    return matches if matches else [text]

def mutate(existing_solution):
    """
    Generates an improved solution for a given problem based on an existing one.
    """
    messages = [
        {"role": "system", "content": MUTATE_PROMPT},
        {"role": "user", "content": f"{PROBLEM_STATEMENT}\n\nExisting solution:\n{existing_solution}\n\nImprove or mutate the given solution to make it better."}
    ]
    print(messages)
    ans = call_model(messages)
    print(ans)
    return extract_code(ans)

def reproduce(solution_a, solution_b):
    """
    Combines two solutions to generate a potentially superior hybrid solution.
    """
    messages = [
        {"role": "system", "content": REPRODUCE_PROMPT},
        {"role": "user", "content": f"{PROBLEM_STATEMENT}\n\nSolution A:\n{solution_a}\n\nSolution B:\n{solution_b}\n\nCombine and refine these two into one optimized solution."}
    ]
    return extract_code(call_model(messages))

# Example usage
if __name__ == "__main__":
    sol1 = ""
    sol2 = ""

    print("=== MUTATION ===")
    print(mutate(sol1))
    print("\n=== REPRODUCTION ===")
    print(reproduce(sol1, sol2))

