import os
import pandas as pd

def combinar_csvs(paths_pastas, arquivo_saida):
    # Lista para armazenar DataFrames
    dataframes = []
    
    # Itera sobre cada pasta
    for path_pasta in paths_pastas:
        # Verifica se o caminho da pasta existe
        if not os.path.isdir(path_pasta):
            print(f"Pasta não encontrada: {path_pasta}")
            continue
        
        # Itera sobre os arquivos dentro da pasta
        for arquivo in os.listdir(path_pasta):
            # Verifica se é um arquivo CSV
            if arquivo.endswith('.csv'):
                caminho_csv = os.path.join(path_pasta, arquivo)
                print(f"Lendo arquivo: {caminho_csv}")
                try:
                    # Lê o CSV e armazena no DataFrame
                    df = pd.read_csv(caminho_csv)
                    dataframes.append(df)
                except Exception as e:
                    print(f"Erro ao ler o arquivo {caminho_csv}: {e}")

    # Concatena todos os DataFrames
    if dataframes:
        df_combined = pd.concat(dataframes, ignore_index=True)
        # Salva o DataFrame combinado em um arquivo CSV de saída
        df_combined.to_csv(arquivo_saida, index=False)
        print(f"Arquivo combinado salvo em: {arquivo_saida}")
    else:
        print("Nenhum arquivo CSV encontrado nas pastas fornecidas.")

if __name__ == "__main__":
    # Exemplo de uso
    paths_pastas = [
        "/home/ricardo/Downloads/multiway/testes/kruskalpert_1271039_2024_10_28",
        "/home/ricardo/Downloads/multiway/testes/kruskalpert_1328963_2024_10_28",
        "/home/ricardo/Downloads/multiway/testes/kruskalpert_1694354_2024_10_28",
        "/home/ricardo/Downloads/multiway/testes/kruskalpert_1884283_2024_10_28",
        "/home/ricardo/Downloads/multiway/testes/kruskalpert_1887782_2024_10_28",
    ]
    
    arquivo_saida = "/home/ricardo/Downloads/multiway/krp_all_results_seeds_600_warm.csv"
    combinar_csvs(paths_pastas, arquivo_saida)