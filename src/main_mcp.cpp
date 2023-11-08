#include "mcp/mcp_instance.hpp"
#include "decoders/mcp_decoder_threshould.hpp"
#include "decoders/mcp_decoder_multiple_threshoulds.hpp"
#include "brkga_mp_ipr.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

// #define single_t
#define multi_t


//-------------------------------[ Main ]------------------------------------//

int main(int argc, char* argv[])
{
    if(argc < 2) {
        cerr
        << "Usage: " << argv[0]
        << " <seed> <config-file> <maximum-running-time>"
        << " <tsp-instance-file>"
        << endl;
        return 1;
    }

    try {
        ////////////////////////////////////////
        // Read command-line arguments and the instance
        ////////////////////////////////////////

        const unsigned seed = stoi(argv[1]);
        const string config_file = argv[2];
        const string instance_file = argv[4];
        const unsigned num_threads = 1;

        cout << "Reading data..." << endl;
        auto instance = MCP_Instance(instance_file);

        // instance.show();

        ////////////////////////////////////////
        // Read algorithm parameters
        ////////////////////////////////////////

        cout << "Reading parameters..." << endl;

        auto [brkga_params, control_params] =
            BRKGA::readConfiguration(config_file);

        // Overwrite the maximum time from the config file.
        control_params.maximum_running_time = chrono::seconds {stoi(argv[3])};

        ////////////////////////////////////////
        // Build the BRKGA data structures
        ////////////////////////////////////////

        cout << "Building BRKGA data and initializing..." << endl;

        # if defined single_t
        MCP_Decoder decoder(instance);

        unsigned chromossome_size = (instance.num_nodes * (instance.num_nodes - 1)) / 2;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );
        # endif

        # if defined multi_t
        MCP_Decoder_Multiple_Threshoulds decoder(instance);

        unsigned chromossome_size = instance.num_nodes + 1;

        BRKGA::BRKGA_MP_IPR<MCP_Decoder_Multiple_Threshoulds> algorithm(
            decoder, BRKGA::Sense::MINIMIZE, seed,
            chromossome_size, brkga_params, num_threads
        );
        # endif

        ////////////////////////////////////////
        // Find good solutions / evolve
        ////////////////////////////////////////

        cout << "Running for " << control_params.maximum_running_time << "..."
             << endl;

        const auto final_status = algorithm.run(control_params, &cout);

        cout
        << "\nAlgorithm status: " << final_status
        << "\n\nBest cost: " << final_status.best_fitness
        << endl;
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