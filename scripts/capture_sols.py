import re
import os
import csv
from statistics import mean

# Função para processar os arquivos de log
def process_log(file_path):
    # Dicionário para armazenar os resultados por instância
    results = {}

    # Regex para encontrar as instâncias
    instance_regex = re.compile(r"/home/ricardo/Downloads/artigo/MultiwayCut/data/concefiles/(.+)\.gr")
    
    # Regex para encontrar os resultados dos algoritmos
    algo_regex = {
        'Cplex': re.compile(r"Cplex: The weight of the multiway cut: ([\d,.]+)"),
        'Simplex': re.compile(r"Simplex: The weight of the multiway cut: ([\d,.]+)"),
        'Isolation': re.compile(r"Isolation: The weight of the multiway cut: ([\d,.]+)"),
        'Local Search': re.compile(r"Local Search: The weight of the multiway cut: ([\d,]+)"),
        'Calinescu': re.compile(r"Calinescu: The weight of the multiway cut: ([\d,]+)"),
        'Exponential Clocks': re.compile(r"Clocks: The weight of the multiway cut: ([\d,]+)"),
        'Buchbinder': re.compile(r"Buchbinder: The weight of the multiway cut: ([\d,]+)"),
        'Single Threshold 1': re.compile(r"Single 1: The weight of the multiway cut: ([\d,]+)"),
        'Single Threshold 2': re.compile(r"Single 2: The weight of the multiway cut: ([\d,]+)"),
        'Descending Threshold 1': re.compile(r"Descending 1: The weight of the multiway cut: ([\d,]+)"),
        'Descending Threshold 2': re.compile(r"Descending 2: The weight of the multiway cut: ([\d,]+)"),
        'Independent Threshold': re.compile(r"Independent: The weight of the multiway cut: ([\d,]+)"),
        '3-Ingredient Mix': re.compile(r"3 Mix: The weight of the multiway cut: ([\d,]+)"),
        '4-Ingredient Mix': re.compile(r"4 Mix: The weight of the multiway cut: ([\d,]+)")
    }

    # Regex para encontrar iterações do Local Search (se houver várias)
    iterations_regex = re.compile(r"Iterations: (\d+)")

    current_instance = None
    local_search_costs = []

    # Abrir o arquivo de log
    with open(file_path, 'r') as file:
        for line in file:
            # Encontrar a instância
            instance_match = instance_regex.search(line)
            if instance_match:
                current_instance = instance_match.group(1)
                results[current_instance] = {algo: [] for algo in algo_regex}
                local_search_costs = []  # Resetar os custos para nova instância

            # Verificar e armazenar os custos dos algoritmos
            if current_instance:
                for algo, regex in algo_regex.items():
                    algo_match = regex.search(line)
                    if algo_match:
                        cost = float(algo_match.group(1).replace('.', '').replace(',', '.'))
                        if algo == 'Local Search':
                            local_search_costs.append(cost)
                        else:
                            results[current_instance][algo].append(cost)

                # Coletar médias das iterações do Local Search
                if local_search_costs:
                    results[current_instance]['Local Search'].append(mean(local_search_costs))
                    local_search_costs = []  # Resetar após coletar a média

    return results

# Função para salvar os resultados em um CSV
def save_to_csv(results, output_file):
    # Cabeçalho do CSV
    header = ['Instance'] + list(next(iter(results.values())).keys())

    # Abrir o arquivo para escrita
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(header)

        # Escrever cada instância e os valores dos algoritmos
        for instance, algos in results.items():
            row = [instance] + [mean(costs) if costs else 'N/A' for costs in algos.values()]
            writer.writerow(row)

# Processar todos os arquivos em um diretório
def process_directory(directory):
    all_results = {}

    for filename in os.listdir(directory):
        if filename.endswith(".sol"):  # Supondo que os arquivos de log terminem com .log
            file_path = os.path.join(directory, filename)
            file_results = process_log(file_path)
            all_results.update(file_results)

    # Salvar os resultados em CSV
    save_to_csv(all_results, 'multiway_cut_results.csv')

# Caminho do diretório com os arquivos de log
log_directory = "/home/ricardo/Downloads/multiway/data/raw_results"

# Processar o diretório
process_directory(log_directory)
