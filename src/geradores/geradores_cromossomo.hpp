#ifndef MCP_GERADORES_CROMO_
#define MCP_GERADORES_CROMO_

#include "brkga_mp_ipr/chromosome.hpp"
#include <iostream>


using namespace std;


BRKGA::Chromosome gerador_cromossomo_th3(string filename);

BRKGA::Chromosome gerador_cromossomo_krp(string filename);

BRKGA::Chromosome gerador_cromossomo_col3(string filename);

#endif