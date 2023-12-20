#include "execute_decoders.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

//-------------------------------[ Main ]------------------------------------//

int main(int argc, char* argv[])
{
    if(argc < 7) {
        cerr
        << "Usage: " << argv[0]
        << " <seed> <config-file> <maximum-running-time>"
        << " <mcp-instance-file> <type-file> <decoder-name>"
        << " <output-file>"
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
        const unsigned type_file = stoi(argv[5]);
        const string decoder_name = argv[6];
        const string output_file_name = argv[7];
        const unsigned num_threads = 1;

        if (decoder_name == "max_flow") {

        }
        else if (decoder_name == "single_t") {
            execute_single_t(seed, config_file, max_run_time, instance_file, type_file, output_file_name, num_threads);
        }
        else if (decoder_name == "multiple_t") {
            execute_multiple_t(seed, config_file, max_run_time, instance_file, type_file, output_file_name, num_threads);
        }
        else if (decoder_name == "kruskal") {
            execute_kruskal(seed, config_file, max_run_time, instance_file, type_file, output_file_name, num_threads);
        }
        else if (decoder_name == "kruskal_pert") {
            execute_kruskal_pert(seed, config_file, max_run_time, instance_file, type_file, output_file_name, num_threads);
        }
        else if (decoder_name == "cuts") {
            execute_cuts(seed, config_file, max_run_time, instance_file, type_file, output_file_name, num_threads);
        }
        else {
            cerr
            << "Specified decoder not listed: "
            << "<single_t> <multiple_t> <kruskal> <kruskal_pert> <cuts>"
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