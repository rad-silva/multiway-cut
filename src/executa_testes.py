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
from pathlib import Path
from glob import glob


#---------------------[Parâmetros para alteração]---------------------------------

# ["threshold", "coloracao", "kruskal", "kruskalpert", "mcortes", "coloracao2", "threshold2", "coloracao3", "threshold3"]
DECODERS =  ["coloracao3"]

DEFAULT_PATH = Path("/home/ricardo/Downloads/multiway/")
INSTANCES_PATH = DEFAULT_PATH / "instances" / "concentric_inst" / "all"
TESTES_PATH = DEFAULT_PATH / "testes"
EXT_FILE = ".gr"

MAIN_FILE_PATH = DEFAULT_PATH / "src" / "main_mcp"
CONFIG_FILE_PATH = DEFAULT_PATH / "src" / "config.conf"
SEED = "20"
MAX_RUN_TIME = "600"

#---------------------[Execução dos testes automatizados]---------------------------------

def execute_tests():
    '''
    ---
    '''
    date = datetime.now().strftime("%Y_%m_%d")

    instances = glob(str(INSTANCES_PATH / f"*{EXT_FILE}"))
    instances.sort()

    for decoder in DECODERS:
        folder_test = f"{decoder}_{date}"
        script = f"{MAIN_FILE_PATH} {SEED} {CONFIG_FILE_PATH} {MAX_RUN_TIME} $instance {decoder} $teste"

        teste_dir = TESTES_PATH / folder_test
        if not teste_dir.exists():
            print(f"Criando pasta {folder_test}")
            teste_dir.mkdir(parents=True)

        # Executa programa para cada uma das instâncias
        for instance in instances:
            out_file = str(teste_dir / Path(instance).stem) + ".sol"
            comando = script.replace("$instance", str(INSTANCES_PATH / instance))
            comando = comando.replace("$teste", str(TESTES_PATH / folder_test / out_file))
            # comando = comando + f" > {str(Path(instance).stem) + '.sol'}"
            print(comando)
            os.system(comando)

        print()


if __name__ == "__main__":
    execute_tests()
