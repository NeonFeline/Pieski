import requests
import json
import os

# Base URL for OpenRouter API
OPENROUTER_API_URL = "https://openrouter.ai/api/v1/chat/completions"
MODEL = "openrouter/polaris-alpha"

# Retrieve API key from environment
API_KEY = os.getenv("OPENROUTER_API_KEY")
if not API_KEY:
    raise ValueError("Missing OPENROUTER_API_KEY environment variable.")

headers = {
    "Authorization": f"Bearer {API_KEY}",
    "Content-Type": "application/json"
}

def call_model(messages):
    """Helper function to call the model with a given conversation."""
    payload = {
        "model": MODEL,
        "messages": messages,
        "temperature": 0.7
    }

    response = requests.post(OPENROUTER_API_URL, headers=headers, data=json.dumps(payload))
    response.raise_for_status()
    return response.json()["choices"][0]["message"]["content"]

def mutate(problem_statement, existing_solution):
    """
    Generates an improved solution for a given problem based on an existing one.
    """
    messages = [
        {"role": "system", "content": "You are a coding expert that optimizes and refactors code for clarity, efficiency, and performance. OUTPUT ONLY CODE, WITHOUT ANY ADDITIONAL TEXT"},
        {"role": "user", "content": f"Problem statement:\n{problem_statement}\n\nExisting solution:\n{existing_solution}\n\nImprove or mutate the given solution to make it better."}
    ]
    return call_model(messages)

def reproduce(problem_statement, solution_a, solution_b):
    """
    Combines two solutions to generate a potentially superior hybrid solution.
    """
    messages = [
        {"role": "system", "content": "You are a code synthesis expert that merges and evolves multiple solutions into a single, efficient one. OUTPUT ONLY CODE, WITHOUT ANY ADDITIONAL TEXT"},
        {"role": "user", "content": f"Problem statement:\n{problem_statement}\n\nSolution A:\n{solution_a}\n\nSolution B:\n{solution_b}\n\nCombine and refine these two into one optimized solution."}
    ]
    return call_model(messages)

# Example usage
if __name__ == "__main__":
    problem = "Write a Python function to compute the factorial of a number recursively."
    sol1 = "def factorial(n): return 1 if n==0 else n*factorial(n-1)"
    sol2 = "def factorial(n): return n * factorial(n-1) if n>1 else 1"

    print("=== MUTATION ===")
    print(mutate(problem, sol1))
    print("\n=== REPRODUCTION ===")
    print(reproduce(problem, sol1, sol2))
