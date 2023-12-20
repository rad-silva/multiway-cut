#ifndef EXECUTE_DECODERS_HPP_
#define EXECUTE_DECODERS_HPP_

#include <string>

void execute_single_t(
    const unsigned seed, const std::string config_file, const unsigned max_run_time,
    const std::string instance_file, const unsigned type_file, std::string output_file_name,
    const unsigned num_threads
);

void execute_multiple_t(
    const unsigned seed, const std::string config_file, const unsigned max_run_time,
    const std::string instance_file, const unsigned type_file, std::string output_file_name,
    const unsigned num_threads
);

void execute_kruskal(
    const unsigned seed, const std::string config_file, const unsigned max_run_time,
    const std::string instance_file, const unsigned type_file, std::string output_file_name,
    const unsigned num_threads
);

void execute_kruskal_pert(
    const unsigned seed, const std::string config_file, const unsigned max_run_time,
    const std::string instance_file, const unsigned type_file, std::string output_file_name,
    const unsigned num_threads
);

void execute_cuts(
    const unsigned seed, const std::string config_file, const unsigned max_run_time,
    const std::string instance_file, const unsigned type_file, std::string output_file_name,
    const unsigned num_threads
);

#endif
