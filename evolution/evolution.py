import os
import tempfile
from pathlib import Path
import shutil
import json
import sys

# Add parent directory to path to import modules
sys.path.append('../')

from scoring.score import compile_cpp_files, run_tests, cleanup_binaries, save_results
import config

from utils import mutate, reproduce
from evaluate import evaluate_cpp_code_list
import math


class Solution:
    def __init__(self, code=""):
        self.code = code
        self.fitness = None


# --- Ensure required directories exist ---
if not os.path.exists(config.EXAMPLES_DIR):
    os.makedirs(config.EXAMPLES_DIR)

if not os.path.exists(config.EVOLUTION_OUTPUT_DIR):
    os.makedirs(config.EVOLUTION_OUTPUT_DIR)


# --- Load initial population from examples ---
initial_codes = []
for file_name in os.listdir(config.EXAMPLES_DIR):
    if file_name.endswith(".cpp"):
        with open(os.path.join(config.EXAMPLES_DIR, file_name), "r") as f:
            initial_codes.append(f.read())

# Initialize population
population = [Solution(code) for code in initial_codes]
while len(population) < config.POPULATION_SIZE:
    population.append(Solution("// Empty solution"))


# --- Utility: logarithmic mean of test results ---
def log_mean(results):
    log_sums = 0
    count = 0
    for example_results in results.values():
        for test_result in example_results.values():
            if test_result > 0:
                log_sums += math.log(test_result)
                count += 1
    return math.exp(log_sums / count) if count > 0 else 1e18


# --- Evaluate a population ---
def evaluate_population(population, aggregation_function=log_mean):
    code_list = [sol.code for sol in population]
    results = evaluate_cpp_code_list(code_list)

    # Apply aggregation function to each solution's results
    for idx, sol in enumerate(population):
        example_key = f"example_{idx + 1}"
        if example_key in results:
            sol.fitness = aggregation_function({example_key: results[example_key]})
        else:
            sol.fitness = float("inf")  # Penalize if missing results
    return results


# --- Sort population by fitness (lower is better) ---
def sort_population(population):
    population.sort(key=lambda sol: sol.fitness)
    return population


# --- Evolve population through mutation and reproduction ---
def evolve_population(population):
    new_population = []

    # Elitism: carry over the best solution
    new_population.append(population[0])



    # Reproduction
    while len(new_population) < config.POPULATION_SIZE:
        parent_a = population[0]  # Best solution
        parent_b = population[1]  # Second best solution

        offspring_codes = reproduce(parent_a.code, parent_b.code)
        for code in offspring_codes:
            if len(new_population) < config.POPULATION_SIZE:
                new_population.append(Solution(code))

    # Mutation with chance
    for sol in population:
        if len(new_population) >= config.POPULATION_SIZE:
            break
        if random.random() < config.MUTATION_CHANCE:
            mutated_codes = mutate(sol.code)
            for code in mutated_codes:
                if len(new_population) < config.POPULATION_SIZE:
                    new_population.append(Solution(code))



    return new_population[:config.POPULATION_SIZE]


# --- Main evolutionary loop ---
if __name__ == "__main__":

    for gen in range(config.GENERATIONS):
        # Evolve
        population = evolve_population(population)

        print(f"=== Generation {gen + 1} ===")
        results = evaluate_population(population)
        population = sort_population(population)

        print(f"Best fitness: {population[0].fitness}")
        print(f"Best code:\n{population[0].code}")

        # Print finess of all solutions
        print("----------------------------")
        print ("All solutions fitness:")
        for idx, sol in enumerate(population):
            print(f"Solution {idx + 1} fitness: {sol.fitness}")
        print("----------------------------")

        # Save all current generation solutions
        for idx, sol in enumerate(population):
            print(f"Solution {idx + 1} fitness: {sol.fitness}")
            out_path = os.path.join(
                config.EVOLUTION_OUTPUT_DIR,
                f"best_solution_gen{gen + 1}_sol{idx + 1}.cpp",
            )
            with open(out_path, "w") as f:
                f.write(sol.code)

        print("\n")
