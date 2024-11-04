import csv
import re

def process_solution_file(input_file, output_csv):
    with open(input_file, 'r') as file:
        lines = file.readlines()

    data = []
    instance_name = None
    search_count = 0

    cost_60_val = None
    time_60_val = None
    iterations_60_val = None

    for i, line in enumerate(lines):
        # Verifica o nome da instância
        instance_match = re.search(r"Executando instancia (\S+)", line)
        if instance_match:
            instance_name = instance_match.group(1)
            print(instance_name)

        # Verifica Local Search e captura dados de 60 segundos
        if "Local Search (epsilon: 1,000)" in line:
            search_count += 1  # Conta o número de execuções Local Search para cada instância

        # Captura as informações para 60 segundos
        if "60 seconds reached:" in line:
            cost_60 = re.search(r"Cost:\s([\d.]+)", lines[i+1])
            time_60 = re.search(r"Time:\s([\d.]+)", lines[i+1])
            iterations_60 = re.search(r"Iterations:\s(\d+)", lines[i+1])
            if cost_60 and time_60 and iterations_60:
                cost_60_val = float(cost_60.group(1))
                time_60_val = float(time_60.group(1))
                iterations_60_val = int(iterations_60.group(1))
            print(line)
            # print(cost_60_val, time_60_val, iterations_60_val)

        # Captura dados de otimização máxima alcançada
        if "600 seconds reached:" in line:
            print(line)
            cost_opt = re.search(r"Cost:\s([\d.]+)", lines[i+2])
            time_opt = lines[i+4].strip("ms\n")
            iterations_opt = re.search(r"Iterations:\s(\d+)", lines[i+2])
            
            if cost_opt and time_opt and iterations_opt:
                cost_opt_val = float(cost_opt.group(1))
                time_opt_val = float(time_opt)
                iterations_opt_val = int(iterations_opt.group(1))

                # Salva os dados para o CSV
                data.append({
                    "instance": instance_name,
                    "search_count": search_count,
                    "cost_60": cost_60_val,
                    "time_60": time_60_val,
                    "iterations_60": iterations_60_val,
                    "cost_opt": cost_opt_val,
                    "time_opt": time_opt_val,
                    "iterations_opt": iterations_opt_val
                })

                if search_count == 3:
                    instance_name = None
                    search_count = 0
                cost_60_val = None
                time_60_val = None
                iterations_60_val = None
                cost_opt_val = None
                time_opt_val = None
                iterations_opt_val = None



    # Escreve os dados no arquivo CSV
    with open(output_csv, 'w', newline='') as csvfile:
        fieldnames = ["instance", "search_count", "cost_60", "time_60", "iterations_60", "cost_opt", "time_opt", "iterations_opt"]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(data)

# Exemplo de uso
process_solution_file('/home/ricardo/Downloads/multiway/local_serach_results.txt', 'local_search_ajustado.csv')
