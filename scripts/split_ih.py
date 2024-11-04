import os

# Lista com os nomes das instâncias na ordem das soluções
nomes_instancias = [
    'N1280K80M4N1.gr', 'N1280K80M4N2.gr', 'N1280K80M4N3.gr', 'N1280K80M4N4.gr', 'N1280K80M4N5.gr',
    'N160K20M4N1.gr', 'N160K20M4N2.gr', 'N160K20M4N3.gr', 'N160K20M4N4.gr', 'N160K20M4N5.gr',
    'N2560K160M4N1.g', 'N2560K160M4N2.g', 'N2560K160M4N3.g', 'N2560K160M4N4.g', 'N2560K160M4N5.g',
    'N320K40M4N1.gr', 'N320K40M4N2.gr', 'N320K40M4N3.gr', 'N320K40M4N4.gr', 'N320K40M4N5.gr',
    'N5120K320M4N1.g', 'N5120K320M4N2.g', 'N640K80M4N1.gr', 'N640K80M4N2.gr', 'N640K80M4N3.gr',
    'N640K80M4N4.gr', 'N640K80M4N5.gr'
]

# Caminho para o arquivo contendo as soluções
caminho_arquivo_solucoes = "/home/ricardo/Downloads/multiway/src/ih.txt"

# Lê o conteúdo do arquivo
with open(caminho_arquivo_solucoes, "r") as arquivo:
    conteudo = arquivo.read()

# Divide o conteúdo em uma lista, onde cada elemento é uma solução
solucoes = conteudo.split("Executing Decoder <mcortes2>")[1:]  # Ignora o primeiro elemento vazio

# Verifica se há a quantidade correta de soluções
if len(solucoes) != len(nomes_instancias):
    print("A quantidade de soluções não corresponde ao número de instâncias.")
else:
    # Salva cada solução em um arquivo separado
    for i, solucao in enumerate(solucoes):
        nome_arquivo_saida = nomes_instancias[i].replace(".gr", ".sol").replace(".g", ".sol")
        caminho_saida = os.path.join("/home/ricardo/Downloads/multiway/data/ih_sols_new2/", nome_arquivo_saida)

        linhas = solucao.strip().splitlines()
        linhas_filtradas = linhas[4:]
        
        with open(caminho_saida, "w") as arquivo_saida:
            arquivo_saida.write("\n".join(linhas_filtradas) + "\n")
        
        print(f"Solução salva em: {caminho_saida}")
