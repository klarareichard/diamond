//
// Created by Klara Reichard on 24.07.18.
//
//
// Created by Klara Reichard on 19.07.18.
//

#include "raw_scoremat.h"

/** Entries for the BLOSUM90 matrix at a scale of ln(2)/2.0. */

static const TNCBIScore s_MaskingScoreMatrix[5 * 5] = {
        /*A,  C,  G,  T,  N*/
        /*A*/    1, -1, -1, -1, -1,
        /*C*/   -1,  1, -1, -1, -1,
        /*G*/   -1, -1,  1, -1, -1,
        /*T*/   -1, -1, -1,  1, -1,
        /*N*/   -1, -1, -1, -1,  1
};
const SNCBIPackedScoreMatrix MaskingScoreMatrix = {
        "ACGTN",
        s_MaskingScoreMatrix,
        -1
};
