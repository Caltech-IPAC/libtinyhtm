/** \file
    \brief      A (non-reentrant) PRNG for test cases

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
 */

#ifndef HTM_RAND_H
#define HTM_RAND_H

#include "tinyhtm/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  Seeds the random number generator.
 */
void htm_seed(unsigned long seed);

/*  Returns a random number in the range [0, 1).
 */
double htm_rand(void);

#ifdef __cplusplus
}
#endif

#endif /* HTM_RAND_H */

