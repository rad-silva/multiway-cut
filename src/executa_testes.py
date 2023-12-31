'''
  Testes automatizados
    Executa o programa principal especificando o decoder e a instância do grafo.
  
  Inputs
    - Requer uma lista de decoders, uma lista de instâncias e a extensão dos
      arquivos de instâncias utilizados.
    - Você deve adicionar manualmente na variável 'method' os decoders que deseja
      rodar no lote de testes atual e na variável 'extension_file' a extensão dos
      arquivos de instância utilizados.
    - A lista de instâncias é criada automaticamente lendo da pasta 'instances'
      apenas os arquivos de extensão 'extension_file' e armazenando na variável
      'instances_names'.

  Outputs
    Cria na pasta 'testes' uma pasta (se não existir) para cada decoder e
    salva o resultado de cada teste realizado.
  
  Observações
    A convenção de nomenclatura de pasta dos testes usa o nome do decoder e a data
    do momento em que o teste é iniciado. Testar múltiplas vezes um decoder na
    data irá sobrescrever o resultado obtido anterior. 
'''

import os
from datetime import datetime


# ["single_t", "multiple_t", "kruskal", "kruskal_pert", "cuts"]
method = ["single_t", "multiple_t", "kruskal", "kruskal_pert"]
extension_file = ".txt"


# Data atual do sistema
data_atual = datetime.now()

formato_data = "%Y_%m_%d"
date = data_atual.strftime(formato_data)


# Diretórios utilizados
atual_path = os.path.dirname(os.path.abspath(__file__))
instances_path = os.path.abspath(os.path.join(atual_path, os.pardir)) + "/instances"
testes_path =  os.path.abspath(os.path.join(atual_path, os.pardir)) + "/testes"


# Argumentos de execução
# <main-file> <seed> <config-file> <maximum-running-time> <mcp-instance-file> <method> <result-file>
main_file = atual_path + "/main_mcp"
seed = "20"
config_file = atual_path + "/config.conf"
maximum_run_time = "30"
type_file = "1"

# Coleta as instâncias de <instances_path>
instances_names = [f for f in os.listdir(instances_path) if os.path.isfile(os.path.join(instances_path, f))]
instances_names = [instance for instance in instances_names if extension_file in instance]
instances_names.sort()


for mtd in method:
  teste_name = mtd + "_" + date
  mainScript = main_file + " " + seed + " " + config_file + " " \
              + maximum_run_time + " " + "$instance" + " " + type_file + " " + mtd + " " + "$teste"


  # Cria a pasta de teste para o método a ser executado
  if not os.path.exists(os.path.join(testes_path, teste_name)):
    print("Criando folder " + teste_name)
    os.makedirs(os.path.join(testes_path, teste_name))

  # Executa programa para cada uma das instâncias
  for i in instances_names:
    comando = mainScript.replace("$instance", instances_path + "/" + i)
    comando = comando.replace("$teste", testes_path + "/" + teste_name + "/" + i)

    print(comando)
    os.system(comando)
    
    print()

    