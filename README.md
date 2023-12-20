# Multiway Cut Problem (MCP)
Uma coleção de algoritmos que utilizam a meta-heurístics BRKGA para encontrar uma boa solução para o Multiway Cut Problem (ou Multiterminals Cut Problem)


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

  
## Compilação
Para compilar o projeto, você pode usar o utilitário `make`.
Ele compilará todos os arquivos necessários.
Certifique-se de que você está no diretório raiz do projeto antes de executar o seguinte comando:
```
make
```
Se, em algum momento, você quiser apagar os arquivos compilados para recompilar o projeto, você pode usar o seguinte comando:
```
make clean
```


## Execução manual
Após a compilação bem-sucedida, você pode executar o programa da seguinte maneira:
```
bin/main instances/{nome da instância} {sigla do algoritmo}
```
Substitua {nome da instância} pelo nome da instância desejada e {sigla do algoritmo} pela sigla do algoritmo que você deseja executar.


## Execução automatizada
Para executar testes com vários algoritmos, certifique-se de que você possui o Python 3 instalado em seu sistema.
Em seguida, execute o seguinte comando:
```
python3 apps/executaTestes.py
```
Certifique-se de que tenha especificado as siglas dos algoritmos a serem executados no arquivo executaTestes.py antes de rodar o comando.


---


Isso é tudo! Agora você está pronto para compilar e executar seu projeto em C++ no Linux.
Se precisar de mais informações ou tiver alguma dúvida, sinta-se à vontade para entrar em contato conosco.
