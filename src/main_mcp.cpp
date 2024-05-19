#include "execute_decoders.hpp"
// #include "executors/execute_single_t.hpp"
// #include "executors/execute_multi_t.hpp"
// #include "executors/execute_kruskal.hpp"
// #include "executors/execute_kruskal_pert.hpp"
// #include "executors/execute_cuts.hpp"
#include "mcp/mcp_instance.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

//-------------------------------[ Main ]------------------------------------//

int main(int argc, char* argv[])
{
    if(argc < 6) {
        cerr
        << "Usage: " << argv[0]
        << " <seed> <config-file> <maximum-running-time>"
        << " <mcp-instance-file> <decoder-name> <output-file>"
        << endl;
        return 1;
    }

    try {
        ////////////////////////////////////////
        // Read command-line arguments and the instance
        ////////////////////////////////////////
        const unsigned seed = stoi(argv[1]);
        const string config_file = argv[2];
        const unsigned max_run_time = stoi(argv[3]);
        const string instance_file = argv[4];
        const string decoder_name = argv[5];
        const string output_file_name = argv[6];
        const unsigned num_threads = 1;

        // MCP_Instance instance(instance_file);

        if (decoder_name == "threshold") {
            execute_threshold(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "coloracao") {
            execute_coloracao(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "kruskal") {
            execute_kruskal(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "kruskalpert") {
            execute_kruskal_pert(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "mcortes") {
            execute_multiplos_cortes(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "coloracao2") {
            execute_coloracao2(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "threshold2") {
            execute_threshold2(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else if (decoder_name == "coloracao3") {
            execute_coloracao3(seed, config_file, max_run_time, instance_file, output_file_name, num_threads);
        }
        else {
            cerr
            << "Specified decoder not listed: "
            << "<threshold> <coloracao> <kruskal> <kruskalpert> <mcortes> <coloracao2> <threshold2> <coloracao3>"
            << endl;
        }
    }
    catch(exception& e) {
        cerr
        << "\n" << string(40, '*') << "\n"
        << "Exception Occurred: " << e.what()
        << "\n" << string(40, '*')
        << endl;
        return 1;
    }

    return 0;
}