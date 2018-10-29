/*
 * Implement the Gibbs sampler for unsupervised motif finding.
 * Author: Yi Zhou
 */
#include "gibbs.h"
using namespace std;

const static size_t INIT_SEED = 20, MAX_ITER = 1000;
// chance of shifting left & right in each iteration
const static double PROB_SHIFT = 0.20, PROB_CHANGE_LEN = 0.01;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf(
            "[ERROR] %s requires 2 arguments, but %d were given.\n"
            "Usage: %s <sequences.fasta> <motif-length>\n"
            "Example: %s data/E.coliRpoN-sequences-16-100nt.fasta 20\n",
            argv[0], argc - 1, argv[0], argv[0]);
        return 1;
    }
    random_device rn;
    // Read fasta file into matrix of strings, where:
    //     the first column is the name, and
    //     the second column is the sequence
    const Matrix<string> fasta = infiles::read_fasta(argv[1]);

    // Global parameters
    size_t motif_len = stoul(argv[2]);
    uniform_real_distribution<double> shift_left_right(0, 1);

    const size_t fasta_size = fasta.size();
    vector<float> max_scores(INIT_SEED);
    Matrix<size_t> final_positions(INIT_SEED);
#pragma omp parallel for
    for (size_t init_stat = 0; init_stat < INIT_SEED; init_stat++) {
        double max_score = 0.0;
        vector<double> fasta_scores(fasta_size, 0.0);
        vector<size_t> final_position;

        // Initialization - randomly select a motif in each sequence
        mt19937 generator{rn()};
        vector<size_t> motif_positions =
            gibbs::init_motif_positions(fasta, motif_len, generator);

        size_t iter_num = 0, num_changes = 1;

        while (num_changes != 0 && iter_num <= MAX_ITER) {
            iter_num++;
            // Update motif position in each sequence to the position with the
            // highest score
            num_changes = gibbs::update_position(
                fasta, motif_len, motif_positions, fasta_scores, generator);
            gibbs::update_final_score(fasta_scores, motif_positions, max_score,
                                      final_position);

            // Shift left or right with a probability
            if (shift_left_right(generator) <= PROB_SHIFT) {
                num_changes++;
                gibbs::shift_left(fasta, motif_len, 1, motif_positions,
                                  fasta_scores);
                gibbs::update_final_score(fasta_scores, motif_positions,
                                          max_score, final_position);

                gibbs::shift_right(fasta, motif_len, 2, motif_positions,
                                   fasta_scores);
                gibbs::update_final_score(fasta_scores, motif_positions,
                                          max_score, final_position);
            }
        }
        for (size_t i = 0; i < fasta_size; i++) {
            gibbs::final_scan(fasta, motif_len, motif_positions, fasta_scores);
        }
        gibbs::update_final_score(fasta_scores, motif_positions, max_score,
                                  final_position);
        max_scores[init_stat] = max_score;
        final_positions[init_stat] = final_position;
    }

    double max_score = 0.0;
    vector<size_t> final_position;
    for (size_t i = 0; i < INIT_SEED; i++) {
        if (max_scores[i] > max_score) {
            max_score = max_scores[i];
            final_position = final_positions[i];
        }
    }

    // Print results
    printf(
        "\nGibbs motif sampler output:\n\n"
        "\tInput file          : %s\n"
        "\tInitial motif length: %s\n"
        "\tFinal motif length  : %zu\n"
        "\tFinal score         : %.6f\n\n",
        argv[1], argv[2], motif_len, max_score);

    printf("Motif sequences and locations:\n\n");
    for (size_t i = 0; i < fasta_size; i++) {
        const size_t motif_pos = final_position[i];
        const string motif_seq = fasta[i][1].substr(motif_pos, motif_len);
        printf("%s\t%zu-%zu\t%s\n", motif_seq.c_str(), motif_pos + 1,
               motif_pos + motif_len, fasta[i][0].c_str());
    }
    printf("\n\n");
    return 0;
}
