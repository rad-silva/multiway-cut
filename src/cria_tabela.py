'''
  Automatiza a criação de um arquivo csv
  a partir de arquivos de solução
'''

import os
from os.path import isfile, join


TESTE_PATH = "/home/ricardo/Downloads/multiway//testes"
EXTENSION_FILE = ".sol"

def create_table():
    '''
        Cria um arquivo csv para cada pasta de teste
        contendo os dados de cada instância testada 

        .
        ├── TEST_PATH
        │   ├── Test1
        │   │   ├── instance1
        │   │   ├── instance2
        │   │   ├── instancen
        │   ├── Test2
        │   │   ├── instance1
        │   │   ├── instance2
        │   │   ├── instancen
    '''

    # Lista todos os diretórios de testes contidos no diretorio <TESTE_PATH>
    testes_names = [
        d
        for d in os.listdir(TESTE_PATH)
        if os.path.isdir(join(TESTE_PATH, d))
    ]

    for teste_name in testes_names:

        # Lista todos os arquivos presentes em /TESTE_PATH/test_name
        sol_files_names = [
            f
            for f in os.listdir(TESTE_PATH + "/" + teste_name)
            if isfile(join(TESTE_PATH+ "/" + teste_name, f))
        ]

        # Mantém apenas os arquivos com extensão <EXTENSION_FILE>
        sol_files_names = [
            instance
            for instance in sol_files_names
            if EXTENSION_FILE in instance
        ]

        # Ordena os arquivos em ordem crescente
        sol_files_names.sort()

        # Extrai o nome do decoder utilizado no teste
        decoder_name = teste_name.split('_')[0]

        print(
            "--" + teste_name + "\n"
            "Instance,Nodes,Edges,Terminals,Valid Solution,"
            "Best cost,Num edges cut,Last update iteration,"
            "Last update time,Final iteration,Final time"
        )

        # Cria e abre o arquivo decoder_name.csv no diretório do teste
        data_file_name = TESTE_PATH + "/" + teste_name + "/" + decoder_name + ".csv"
        data_file = open(data_file_name, "w", encoding="utf-8")

        # Grava a linha de cabeçalho no arquivo
        data_file.write(
            "Instance,Nodes,Edges,Terminals,Valid Solution,"
            "Best cost,Num edges cut,Last update iteration,"
            "Last update time,Final iteration,Final time\n"
        )

        for i in sol_files_names:

            instance_name = i.split(".")[0]

            path_sol_file_name = TESTE_PATH + "/" + teste_name + "/" + i
            file_solution = open(path_sol_file_name, "r", encoding="utf-8")

            # Aramazena todas as linhas do arquivo em uma lista
            solucao = file_solution.readlines()

            # Considera apenas as primeiras 10 linhas do arquivo
            solucao = solucao[:10]

            # Extrai o conteúdo de cada linha do arquivo (título:conteúdo)
            data = [instance_name] + [
                line.strip().split(": ")[1]
                for line in solucao
                if ":" in line
            ]

            # Grava todo o conteúdo separado por vírgula em uma linha no arquivo de saída
            data_file.write(",".join(data) + "\n")

            print(",".join(data))

        print()
        data_file.close()

if __name__ == "__main__":
    create_table()
