#pragma once

#include <string>
#include <vector>
#include <sstream> // std::stringstream
#include <fstream> // std::ifstream
#include <cstdio>

namespace nw
{
std::string read_fasta(const std::string input_file);
void strip_non_alphabetic(std::string &s);
void to_upper_case(std::string &s);
void reverse_string(std::string &s);
void prepare_sequence(std::string &s);
void initialize_score_matrix(std::vector<std::vector<float>> &m, float gap_score);
float score_top_left(std::vector<std::vector<float>> &m,
                     size_t i, size_t j,
                     std::string &seq1, std::string &seq2,
                     float match_score, float mismatch_score);
float score_left(std::vector<std::vector<float>> &m,
                 size_t i, size_t j, float gap_score);
float score_top(std::vector<std::vector<float>> &m,
                size_t i, size_t j, float gap_score);
float max_score(float, float, float);
} // namespace nw
