
import os
import re
import csv

# Diretório onde estão os arquivos .sol
pasta_sol = "/home/ricardo/Downloads/multiway/data2/basic_results/"

# Nome do arquivo CSV de saída
arquivo_saida_csv = "/home/ricardo/Downloads/multiway/data2/basic_results/basics.csv"


def buscar_valor(pattern, linha, tipo=int):
    """Função auxiliar para extrair valores usando uma expressão regular."""
    match = re.search(pattern, linha)
    if match:
        return tipo(match.group(1))
    return None


# Função para extrair dados do conteúdo de cada arquivo .sol
def extrair_dados_instancia(lines):
    dados = {}
    
    # Atributos da instância
    for i in range(len(lines)):
        if lines[i].startswith("Vertices:"):
            dados['vertices'] = buscar_valor(r'Vertices: (\d+)', lines[i])
            dados['arestas'] = buscar_valor(r'Edges: (\d+)', lines[i])
            dados['terminais'] = buscar_valor(r'K: (\d+)', lines[i])
            break
    
    i = 0  # Reinicia o índice para a busca dos resultados dos algoritmos
    ls=1
    while i < len(lines):
        # Cplex
        if lines[i].startswith("Cplex: The weight of the multiway cut:"):
            dados['cplex_custo'] = float(lines[i].split(":")[-1].strip().replace(",", "."))
            dados['cplex_tempo (s)'] = int(lines[i+1].split(" ")[-2].strip().replace(",", ".")) / 1_000_000_000.0
        # Simplex
        elif lines[i].startswith("Simplex: The weight of the multiway cut:"):
            dados['simplex_custo'] = float(lines[i].split(":")[-1].strip().replace(",", "."))
            dados['simplex_tempo (s)'] = int(lines[i+1].split(" ")[-2].strip().replace(",", ".")) / 1_000_000_000.0
        # Isolation Heuristic
        elif lines[i].startswith("Isolation: The weight of the multiway cut:"):
            dados['isolation_custo'] = float(lines[i].split(":")[-1].strip().replace(",", "."))
            dados['isolation_tempo (s)'] = int(lines[i+1].split(" ")[-1].strip().replace(",", ".")) / 1_000_000_000.0
        # Local Search
        elif lines[i].startswith("Local Search: The weight of the multiway cut:"):
            dados[f'local_search_custo_{ls}'] = float(lines[i].split(":")[-1].strip().replace(",", "."))
            dados[f'local_search_tempo_{ls} (s)'] = int(lines[i+1].split(" ")[-2].strip().replace(",", ".")) / 1_000_000_000.0
            ls=ls+1
        # Calinescu
        elif lines[i].startswith("Calinescu: The weight of the multiway cut:"):
            dados['calinescu_custo'] = float(lines[i].split(":")[-1].strip().replace(",", "."))
            dados['calinescu_tempo (s)'] = int(lines[i+1].split(" ")[-2].strip().replace(",", ".")) / 1_000_000_000.0
        # Buchbinder
        elif lines[i].startswith("Buchbinder: The weight of the multiway cut:"):
            dados['buchbinder_custo'] = float(lines[i].split(":")[-1].strip().replace(",", "."))
            dados['buchbinder_tempo (s)'] = int(lines[i+1].split(" ")[-2].strip().replace(",", ".")) / 1_000_000_000.0
        
        i += 1  # Avança para a próxima linha
    
    return dados



# Lista para armazenar todos os dados extraídos
dados_csv = []

# Processa cada arquivo .sol na pasta

arquivos = os.listdir(pasta_sol)
arquivos.sort()

for arquivo in arquivos:
    if arquivo.endswith('.sol'):
        caminho_arquivo = os.path.join(pasta_sol, arquivo)
        with open(caminho_arquivo, 'r') as f:
            conteudo = f.readlines()
        
        dados_instancia = extrair_dados_instancia(conteudo)
        dados_instancia['instancia'] = os.path.splitext(arquivo)[0]  # Nome da instância sem extensão
        dados_csv.append(dados_instancia)

# Cabeçalho do CSV
colunas = [
    'instancia', 'vertices', 'arestas', 'terminais',
    'cplex_custo', 'cplex_tempo (s)',
    'simplex_custo', 'simplex_tempo (s)',
    'isolation_custo', 'isolation_tempo (s)',
    'local_search_custo_1', 'local_search_tempo_1 (s)',
    'local_search_custo_2', 'local_search_tempo_2 (s)',
    'local_search_custo_3', 'local_search_tempo_3 (s)',
    'calinescu_custo', 'calinescu_tempo (s)',
    'buchbinder_custo', 'buchbinder_tempo (s)'
]

# Grava os dados no arquivo CSV
with open(arquivo_saida_csv, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=colunas)
    writer.writeheader()
    for dados in dados_csv:
        writer.writerow(dados)

print(f'Arquivo CSV "{arquivo_saida_csv}" gerado com sucesso.')


