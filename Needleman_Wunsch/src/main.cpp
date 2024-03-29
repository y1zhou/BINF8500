/*
 * Implement the Needleman-Wunsch algorithm and align the two given sequences.
 * Author: Yi Zhou
*/
#include "nw.h"

using namespace std;

const unsigned PRINT_WIDTH = 60;

int main(int argc, char **argv)
{
    if (argc != 6)
    {
        printf("[Error] %s takes 5 arguments, but %d were given.\n\n"
               "Usage: %s <seq1> <seq2> <match_score> <mismatch_score> <gap_penalty>\n\n"
               "This program implements the Needleman-Wunsch algorithm and\n"
               "align the two given sequences.\n"
               "Matches and mismatches have uniform scores, whereas gaps have a linear penalty.\n",
               argv[0], argc - 1, argv[0]);
        return 1;
    }
    // Read file in and parse sequence into a single string
    const float MATCH = stof(argv[3]), MISMATCH = stof(argv[4]), GAP = stof(argv[5]);
    string seq1, seq2;
    seq1 = nw::read_fasta(argv[1]);
    seq2 = nw::read_fasta(argv[2]);

    // convert all to upper case and remove non-alphabetic characters
    nw::prepare_sequence(seq1);
    nw::prepare_sequence(seq2);
    const size_t NROW = seq1.length(),
                 NCOL = seq2.length();
    printf("\nSequence 1: (%s, %zu characters):\n\n%s\n\n",
           argv[1], NROW, seq1.c_str());
    printf("\nSequence 2: (%s, %zu characters):\n\n%s\n\n",
           argv[2], NCOL, seq2.c_str());

    // Construct score matrix (reverse the sequence)
    nw::reverse_string(seq1);
    nw::reverse_string(seq2);
    printf("\nScores:\n\tMatch: %.2f\n\tMismatch: %.2f\n\tGap: %.2f (linear)\n\n",
           MATCH, MISMATCH, GAP);
    Matrix matrix(NROW + 1, vector<float>(NCOL + 1));
    nw::initialize_score_matrix(matrix, GAP);

    // Calculate score
    for (size_t i = 1; i <= NROW; i++)
    {
        for (size_t j = 1; j <= NCOL; j++)
        {
            matrix[i][j] = nw::max_score(nw::score_left(matrix, i, j, GAP),
                                         nw::score_top_left(matrix, i, j,
                                                            seq1, seq2,
                                                            MATCH, MISMATCH),
                                         nw::score_top(matrix, i, j, GAP));
        }
    }

    // Trace back and find the alignment
    size_t i = NROW, j = NCOL;
    string res1, res2, alignment;
    const size_t result_len = NROW + NCOL;
    res1.reserve(result_len);
    res2.reserve(result_len);
    alignment.reserve(result_len);
    while (i > 0 && j > 0)
    {
        if (nw::score_left(matrix, i, j, GAP) == matrix[i][j])
        {
            res1 += '-';
            res2 += seq2[j - 1];
            alignment += ' ';
            j--;
        }
        else if (nw::score_top(matrix, i, j, GAP) == matrix[i][j])
        {
            res1 += seq1[i - 1];
            res2 += '-';
            alignment += ' ';
            i--;
        }
        else
        {
            res1 += seq1[i - 1];
            res2 += seq2[j - 1];
            if (seq1[i - 1] == seq2[j - 1])
            {

                alignment += '*';
            }
            else
            {
                alignment += ' ';
            }
            i--;
            j--;
        }
    }
    // Deal with consecutive gaps at the ends of the alignment
    while (j > 0)
    {
        res1 += '-';
        res2 += seq2[j - 1];
        alignment += ' ';
        j--;
    }
    while (i > 0)
    {
        res1 += seq1[i - 1];
        res2 += '-';
        alignment += ' ';
        i--;
    }

    // Print results
    printf("The final alignment score is %.2f\n\n", matrix[NROW][NCOL]);
    const size_t print_len = res1.length();
    size_t seq_pos1 = 1, seq_pos2 = 1;
    for (size_t i = 0; i < print_len; i += PRINT_WIDTH)
    {
        string substring1 = res1.substr(i, PRINT_WIDTH),
               substring2 = alignment.substr(i, PRINT_WIDTH),
               substring3 = res2.substr(i, PRINT_WIDTH);
        printf("%8zu: %s\n", seq_pos1, substring1.c_str());
        printf("%10c%s\n", ' ', substring2.c_str());
        printf("%8zu: %s\n\n", seq_pos2, substring3.c_str());
        seq_pos1 += PRINT_WIDTH - nw::count_gap(substring1);
        seq_pos2 += PRINT_WIDTH - nw::count_gap(substring3);
    }

    return 0;
}
