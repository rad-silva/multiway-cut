import os
from os.path import isfile, join

instances_path = "../instances/steiner"
testes_path = "../testes"
extension_file = ".sol"

# Lista todas os diretórios contidos em <instances_path>
testes_names = [d for d in os.listdir(testes_path) if os.path.isdir(join(testes_path, d))]

# Percorre cada um dos testes
for teste_name in testes_names:
  print("--" + teste_name)

  # Lê todos os arquivos com extensão <extension_file>
  sol_files_names = [f for f in os.listdir(testes_path + "/" + teste_name) if isfile(join(testes_path+ "/" + teste_name, f))]
  sol_files_names = [instance for instance in sol_files_names if extension_file in instance]
  sol_files_names.sort()
  
  decoder_name = teste_name.split('_')[0]

  data_file_name = testes_path + "/" + teste_name + "/" + decoder_name + ".csv"
  data_file = open(data_file_name, "w")

  print("Instance,Nodes,Edges,Terminals,Valid Solution,Best cost,Num edges cut,Last update iteration,Last update time,Final iteration,Final time")
  data_file.write("Instance,Nodes,Edges,Terminals,Valid Solution,Best cost,Num edges cut,Last update iteration,Last update time,Final iteration,Final time\n")

  for i in sol_files_names:

    instance_name = i.split(".")[0]

    path_sol_file_name = testes_path + "/" + teste_name + "/" + i 
    arquivoDeSolucao = open(path_sol_file_name, "r")

    solucao = arquivoDeSolucao.readlines()
    solucao = solucao[:10] # nro de linhas a considerar do arquivo
    
    data = [instance_name] + [line.strip().split(": ")[1] for line in solucao if ":" in line]
    print(",".join(data))
    data_file.write(",".join(data) + "\n")

  print()
  data_file.close()