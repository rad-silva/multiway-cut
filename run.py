import os
import subprocess


# Lista de nomes dos arquivos (à esquerda da igualdade).
nomes_instancias = [
    # 'N160K20M4N1',
    # 'N160K20M4N2',
    # 'N160K20M4N3',
    # 'N160K20M4N4',
    # 'N160K20M4N5',
    # 'N320K40M4N1',
    # 'N320K40M4N2',
    # 'N320K40M4N3',
    # 'N320K40M4N4',
    # 'N320K40M4N5',
    # 'N640K80M4N1',
    # 'N640K80M4N2',
    # 'N640K80M4N3',
    # 'N640K80M4N4',
    # 'N640K80M4N5',
    'N1280K80M4N1',
    'N1280K80M4N2',
    'N1280K80M4N3',
    'N1280K80M4N4',
    'N1280K80M4N5',
    # 'N2560K160M4N1',
    'N2560K160M4N2',
    'N2560K160M4N3',
    'N2560K160M4N4',
    'N2560K160M4N5',
    'N5120K320M4N1',
    'N5120K320M4N2',
]


# Executar o comando para cada instância
# for nome in nomes_instancias:

#     comando = f"/usr/lib/jvm/java-1.11.0-openjdk-amd64/bin/java -javaagent:/snap/intellij-idea-community/537/lib/idea_rt.jar=42643:/snap/intellij-idea-community/537/bin -Dfile.encoding=UTF-8 classpath /home/ricardo/Downloads/m-artigo/MultiwayCut/out/production/MultiwayCut:/home/ricardo/ibm/ILOG/CPLEX_Studio2211/cplex/lib/cplex.jar:/home/ricardo/guava/guava-23.0.jar Main {nome} 1 1 10"

#     # Executa o comando e captura a saída
#     try:
#         print(comando)
#         os.system(comando)
#         print()
#     except Exception as e:
#         print(f"Erro ao executar o comando para {nome}: {e}")
#         raise

java_exec = "/usr/lib/jvm/java-1.11.0-openjdk-amd64/bin/java"
java_agent = "-javaagent:/snap/intellij-idea-community/537/lib/idea_rt.jar=42643:/snap/intellij-idea-community/537/bin"
classpath = "/home/ricardo/Downloads/m-artigo/MultiwayCut/out/production/MultiwayCut:/home/ricardo/ibm/ILOG/CPLEX_Studio2211/cplex/lib/cplex.jar:/home/ricardo/guava/guava-23.0.jar"
main_class = "Main"

for nome in nomes_instancias:
    # Comando único para evitar quebras de linha
    comando = [
        java_exec, java_agent, "-Dfile.encoding=UTF-8",
        "-classpath", classpath,
        main_class, nome, "1", "1", "10"
    ]

    with open("local_serach_results.txt", "a") as f:
    
        try:
            print(f"Executando instancia {nome}\n")
            print(' '.join(comando) + "\n")
            subprocess.run(comando, check=True)

            print("\n\n")

        except subprocess.CalledProcessError as e:
            print(f"Erro ao executar o comando para {nome}: {e}")

            print("\n\n")