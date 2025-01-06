import os
import re
import csv

# Diretório onde estão os arquivos .sol
pasta_sol = "/home/ricardo/Downloads/multiway/data2/complex_results/"
# Nome do arquivo CSV de saída
arquivo_saida_csv = "/home/ricardo/Downloads/multiway/data2/complex_results/complex.csv"


def buscar_valor(pattern, linha, tipo=float):
    """Função auxiliar para extrair valores usando uma expressão regular."""
    match = re.search(pattern, linha)
    if match:
        return tipo(match.group(1).replace(",", "."))
    return None


# Função para extrair dados do conteúdo de cada arquivo .sol
def extrair_dados_instancia(lines):
    dados = {}
    
    # Atributos da instância
    for line in lines:
        if line.startswith("Vertices:"):
            dados['vertices'] = buscar_valor(r'Vertices: (\d+)', line, int)
            dados['arestas'] = buscar_valor(r'Edges: (\d+)', line, int)
            dados['terminais'] = buscar_valor(r'K: (\d+)', line, int)
            break
    
    # Captura os dados da Heurística Isolation
    for i, line in enumerate(lines):
        if line.startswith("Isolation: The weight of the multiway cut:"):
            dados['isolation_custo'] = buscar_valor(r'The weight of the multiway cut: ([\d,]+)', line)
            dados['isolation_tempo (s)'] = buscar_valor(r'Time: (\d+)', lines[i + 1]) / 1_000_000_000.0
            break

    # Captura os dados das três execuções de Local Search
    ls_count = 1
    i = 0
    while i < len(lines) and ls_count <= 3:
        if "Local Search (epsilon:" in lines[i]:
            # Encontrar o custo e o tempo em "60 seconds reached"
            for j in range(i, len(lines)):
                if "60 seconds reached:" in lines[j]:
                    dados[f'local_search_custo_{ls_count}_60s'] = buscar_valor(r'Cost: ([\d.]+)', lines[j + 1])
                    dados[f'local_search_tempo_{ls_count}_60s (s)'] = buscar_valor(r'Time: ([\d.]+)', lines[j + 1]) / 1000.0
                
                # Encontrar o custo e o tempo final (após "max optimization reached" ou "600 seconds reached")
                if "max optimaztion reached:" in lines[j] or "600 seconds reached:" in lines[j]:
                    dados[f'local_search_custo_{ls_count}_final'] = buscar_valor(r'Cost: ([\d.]+)', lines[j + 1])
                    dados[f'local_search_tempo_{ls_count}_final (s)'] = buscar_valor(r'Time: ([\d.]+)', lines[j + 1]) / 1000.0
                    ls_count += 1
                    i = j
                    break
        i += 1
    
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
    'isolation_custo', 'isolation_tempo (s)',
    'local_search_custo_1_60s', 'local_search_tempo_1_60s (s)',
    'local_search_custo_1_final', 'local_search_tempo_1_final (s)',
    'local_search_custo_2_60s', 'local_search_tempo_2_60s (s)',
    'local_search_custo_2_final', 'local_search_tempo_2_final (s)',
    'local_search_custo_3_60s', 'local_search_tempo_3_60s (s)',
    'local_search_custo_3_final', 'local_search_tempo_3_final (s)'
]

# Grava os dados no arquivo CSV
with open(arquivo_saida_csv, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=colunas)
    writer.writeheader()
    for dados in dados_csv:
        writer.writerow(dados)

print(f'Arquivo CSV "{arquivo_saida_csv}" gerado com sucesso.')
