#ifndef EXECUTE_DECODERS_HPP_
#define EXECUTE_DECODERS_HPP_

#include <string>

void execute_threshold(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_coloracao(
    const unsigned seed, const std::string config_file,
    const unsigned max_run_time, const std::string instance_file,
    std::string output_file_name, const unsigned num_threads
);

void execute_kruskal(
    const unsigned seed, const std::string config_file,
    const unsigned max_run_time, const std::string instance_file,
    std::string output_file_name, const unsigned num_threads
);

void execute_kruskal_pert(
    const unsigned seed, const std::string config_file,
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_multiplos_cortes(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_coloracao2(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_threshold2(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_coloracao3(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_threshold3(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

void execute_multiplos_cortes2(
    const unsigned seed, const std::string config_file, 
    const unsigned max_run_time, const std::string instance_file, 
    std::string output_file_name, const unsigned num_threads
);

#endif
