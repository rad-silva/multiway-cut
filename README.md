# Multiway Cut Problem (MCP)
Uma coleção de algoritmos que utilizam a meta-heurística BRKGA para encontrar uma boa solução para o Multiway Cut Problem (ou Multiterminals Cut Problem)


## Formulação do problema
O Multiway Cut Problem (MCP) é uma generalização do clássico Problema do Corte Mı́nimo.
Este problema também é conhecido como Multiterminal Cut. No MCP, dado um grafo não
direcionado G = (V, E), onde cada aresta e ∈ E possui um custo positivo associado c(e), e
um conjunto de vértices distintos conhecidos como terminais S = {s1 , s2 , . . . , sk } ⊆ V , busca-se
encontrar um conjunto F de arestas cuja remoção desconecta todos os k terminais. Em outras
palavras, nenhum par de vértices si e sj (i ≠ j) pode pertencer ao mesmo componente conexo
em (V, E − F). O desafio é determinar o conjunto de arestas cuja remoção cause a desconexão
desejada, enquanto se minimiza o custo total envolvido.


## Ambiente de desenvolvimento
O projeto foi desenvolvido e testado com as seguintes versões de software, mas pode ser compatível com outras versões:

- Ubuntu 22.04.2 LTS
- C++ 12.3.0
- Python 3.10.12
- [BRKGA-MP-IPR 3.0](https://ceandrade.github.io/brkga_mp_ipr_cpp/)

Opcionais:
- openjdk 11.0.24 -----> para utilizar o gerador de instâncias em java
- cplex studio 22.11 -----> para solução de instâncias com o cplex


## Estrutura do projeto

O projeto é estruturado da seguinte forma

```
.
├── data2
│   ├── basic_ih
│   ├── basic_instances
│   ├── basic_results
│   ├── complex_ih
│   ├── complex_instances
│   ├── complex_results
│   └── csvs
├── scripts
├── src
│   ├── brkga_mp_ipr
│   ├── decoders
│   ├── geradores
│   ├── max_flow
│   └── mcp
└── testes
```

- **`data2/`**: contém instâncias com decaimento exponencial e seus respectivos resultados obtidos com os algoritmos do artigo de referência (veja a seção 6.1 do artigo [A local search approximation algorithm for the multiway cut problem](https://www.sciencedirect.com/science/article/pii/S0166218X23002007) para compreensão das instâncias com decaimento exponencial utilizadas como benchmark. O gerador de instâncias foi desenvolvido pelos autores do artigo em linguagem java e pode ser obtido em (https://csd.uwo.ca/~ablochha/MultiwayCut.html) que também contém a implementação de algoritmos para o multiway cut e sua formulação com CPLEX).

Mais detalhes os arquivos de instâncias são encontrados em seus respectivos diretórios nos arquivos nomeados como `infos.txt`

- **`src/brkga_mp_ipr/`**: Contém os importáveis do Multiparent BRKGA com Implict Path Relink.
- **`src/decoders/`**: Códigos fontes do decoders.
- **`src/geradores/`**: Códigos para geração de cromossomos para os principais decoders a partir da solução de uma instância.
- **`src/max_flow/`**: Códigos de um grafo direcionado, algoritmo push-relabel e estrutura de dados utilizadas, para o Maximum FLow Problem, que envolve o cálculo do fluxo máximo e corte mínimo entre dois terminais (desenvolvido em um trabalho anterior).
- **`src/mcp/`**: Códigos de um grafo não direcionado com múltiplos terminais para o Multiay Cut Problem.
- **`testes/`**: Armazena os arquivos de saída dos testes realizados.

## Como executar

<br>

**<u>Compilação</u>**

Para compilar o projeto, é utilizado o arquivo `src/Makefile` que contém todas as importações dos arquivos a serem compilados, flags de otimização, etc.
Certifique-se de que você está no diretório `src/` e execute o seguinte comando:
```
make
```
Se, em algum momento, você quiser apagar os arquivos compilados para recompilar o projeto, você pode usar o seguinte comando:
```
make clean
```
Caso deseje incluir um novo arquivo para compilação, acrescente na variável `OBJS` do arquivo `src/Makefile` o nome do SEUARQUIVO.o (supondo que você tenha codado os arquivos SEUARQUIVO.hpp e SEUARQUIVO.cpp). Você pode acessar o Makefile e visualizar como os arquivos foram importados e os demais recursos. Caso não tenha familiaridade com as diretivas make, você pode colar o conteúdo do arquivo em um chatbot, como o chatGPT e o Gemini, e eles te fornecerão uma explicação detalhada dos comandos e definições.

<br>

**<u>Execução manual de instâncias</u>**

Após a compilação bem-sucedida, você pode executar o programa da seguinte usando a linha de comando da seguinte maneira (certifique-se de estar no diretório `src/`):

```
./main_mcp <seed> <config-file-relative-path> <maximum-running-time-seconds> <mcp-instance-file-relative-path> <decoder-name> <output-file-relative-path>"
```

Abaixo deixamos um exemplo para execução do decoder baseado em coloração de vértices com execução máxima de 10 segundos. Sinta-se à vontade para testar.

```
cd src

./main_mcp 20 ./config.conf 1 ../data2/basic_instances/N160_M2N_K10_1.gr coloracao3 ../testes/N160_M2N_K10_1.sol
```

<br>

**<u>Execução automatizada de instâncias</u>**


Para executar testes com vários algoritmos, certifique-se de que você possui o Python 3 instalado em seu sistema.

O arquivo `src/executa_testes.py` contém um programa python em que você pode especificar todos os parâmetros de execução mostrados anteriormente. A diferença é que pode ser dado um conjunto de nomes de decoders e o caminho relativo de uma pasta de instâncias e ele rodará, para cada decoder, todas as instâncias da pasta. Após ter definido todos os parâmetros, salve o arquivo e execute o seguinte comando:

```
python3 apps/executa_testes.py
```

Para cada instância será gerada uma subpasta em `testes/` nomeada com o nome do decoder e a data de sua execução **(Atenção!!! executar um mesmo decoder na mesma data com o mesmo conjunto de instâncias sobrescreverá o resultado anterior)**. Para cada teste (instância) será gerado um arquivo de saída `.sol` contendo as informações da execução e resultados obtidos.

Você também pode gerar arquivos .csv para cada conjunto de soluções contidas na pasta de testes. Para isso, o arquivo `src/cria_tabela.py` contém um programa python que acessa cada subpastas do diretório `testes/` e cria um arquivo `.csv` onde cada linha corresponde à solução registrada em um arquivo de solução contido na respectiva subpasta. Execute

```
python3 src/cria_tabela.py
```

<br>

---

Isso é tudo! Agora você está pronto para compilar e executar este projeto em C++ no Linux.
Se precisar de mais informações ou tiver alguma dúvida, sinta-se à vontade para entrar em contato conosco.
