//
// Created by Klara Reichard on 19.07.18.
//

#include "raw_scoremat.h"

/** Entries for the BLOSUM90 matrix at a scale of ln(2)/2.0. */

static const TNCBIScore s_NucleotideScoreMatrix[5 * 5] = {
               /*A,  C,  G,  T,  N*/
        /*A*/    2, -3, -3, -3, -3,
        /*C*/   -3,  2, -3, -3, -3,
        /*G*/   -3, -3,  2, -3, -3,
        /*T*/   -3, -3, -3,  2, -3,
        /*N*/   -3, -3, -3, -3,  2
};
const SNCBIPackedScoreMatrix NucleotideScoreMatrix = {
        "ACGTN",
        s_NucleotideScoreMatrix,
        -3
};
