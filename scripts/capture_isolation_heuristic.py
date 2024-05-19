import re
import os
import glob

def extract_isolation_weights(file_path):
    isolation_weights = []

    with open(file_path, 'r') as file:
        for line in file:
            if "Isolation: The weight of the multiway cut" in line:
                match = re.search(r"Isolation: The weight of the multiway cut: (\d+,\d+)", line)
                if match:
                    isolation_weights.append(match.group(1))

    return isolation_weights

def process_all_files_in_directory(directory_path):
    results = []

    files_path = glob.glob(os.path.join(directory_path, '*.sol'))
    files_path.sort()

    # Listar todos os arquivos na pasta especificada
    for file_path in files_path:
        # Obter os valores de isolamento para cada arquivo
        isolation_weights = extract_isolation_weights(file_path)
        if isolation_weights:
            # Armazenar o nome do arquivo e os valores extraídos
            file_results = {
                'file_name': os.path.basename(file_path),
                'weights': isolation_weights
            }
            results.append(file_results)

    return results

def main():
    # Caminho para o diretório que contém os arquivos de dados
    directory_path = '/home/ricardo/Downloads/multiway/instances/concentric_sol/'

    # Processar todos os arquivos no diretório
    results = process_all_files_in_directory(directory_path)

    # Imprimir os resultados
    for result in results:
        print(f"Arquivo: {result['file_name']}")
        for weight in result['weights']:
            print(weight)
        print()  # Linha em branco para separar os resultados dos arquivos

if __name__ == "__main__":
    main()
