import re
import csv
import os

def parse_solution_file(file_path, csv_writer):
    with open(file_path, 'r') as f:
        instance_name = None
        vertices = edges = terminals = None
        local_search_count = 0
        row_data = []

        cut_weight, iterations, time_ms = None, None, None
        lines = f.readlines() 

        for i, line in enumerate(lines):
            print(line)
            
            # Procura pelo nome da instância
            if line.startswith("/"):
                instance_name = os.path.basename(line.strip()).replace(".gr", "")

            # Procura pelos parâmetros do grafo (Vertices, Edges e Terminais)
            vertices_match = re.search(r"Vertices: (\d+), Edges: (\d+), K: (\d+)", line)
            if vertices_match:
                vertices, edges, terminals = vertices_match.groups()

            # Procura pelo peso do corte
            cut_weight_match = re.search(r"Local Search: The weight of the multiway cut: ([\d,]+)", line)
            if cut_weight_match:
                cut_weight = cut_weight_match.group(1).replace(",", ".")
                local_search_count += 1

            # Procura pelo número de iterações
            iterations_match = re.search(r"Iterations: (\d+)", line)
            if iterations_match:
                iterations = iterations_match.group(1)

            # Procura pelo tempo gasto (em ms)
            time_match = re.search(r"(\d+)ms", line)
            if time_match:
                time_ms = time_match.group(1)

            if instance_name and vertices and edges and terminals and cut_weight:
                if i == len(lines) - 1 or re.search(r"Local Search: The weight of the multiway cut: ([\d,]+)", lines[i+1]) or lines[i+1].startswith("****************************************Trial"):
                    row_data = [instance_name, vertices, edges, terminals, f"Local Search {local_search_count}", cut_weight, iterations, time_ms]
                    csv_writer.writerow(row_data)
                    cut_weight, iterations, time_ms = None, None, None
                if i == len(lines) - 1 or lines[i+1].startswith("****************************************Trial"):
                    local_search_count = 0

# Caminho para os arquivos de solução e arquivo CSV de saída
solution_files = [
    '/home/ricardo/Downloads/multiway/instances/concentric_inst/new2/N160K20M4N.sol',
    '/home/ricardo/Downloads/multiway/instances/concentric_inst/new2/N320K40M4N.sol',
    '/home/ricardo/Downloads/multiway/instances/concentric_inst/new2/N640K80M4N.sol',
    '/home/ricardo/Downloads/multiway/instances/concentric_inst/new2/N1280K80M4N1.sol',
    '/home/ricardo/Downloads/multiway/instances/concentric_inst/new2/N2560K160M4N.sol',
    '/home/ricardo/Downloads/multiway/instances/concentric_inst/new2/N5120K320M4N.sol',
]
output_csv = 'multiway_cut_results_local_Search.csv'

# Abre o arquivo CSV para escrita
with open(output_csv, mode='w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    # Escreve o cabeçalho no arquivo CSV
    csv_writer.writerow(["Instance", "Vertices", "Edges", "Terminals", "Algorithm", "Cut Weight", "Iterations", "Time (ms)"])

    # Processa cada arquivo de solução
    for solution_file in solution_files:
        parse_solution_file(solution_file, csv_writer)

print(f"Dados extraídos com sucesso para {output_csv}")


# OneEach
# Clumps
# Isolation Heuristic
# Random
# Random