/** \file
    \brief  Unit tests for HTM implementation

    \authors Serge Monkewitz
    \copyright IPAC/Caltech
  */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>

#include "tinyhtm/htm.h"
#include "cmp.h"
#include "rand.h"


#define HTM_ASSERT(pred, ...) \
    do { \
        if (!(pred)) { \
            fprintf(stderr, "[%s:%d]  ", __FILE__, __LINE__); \
            fprintf(stderr, #pred " is false: " __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            exit(1); \
        } \
    } while(0)

#define SQRT_2_2 0.707106781186547524400844362105 /* sqrt(2)/2 */
#define SQRT_3_3 0.577350269189625764509148780503 /* sqrt(3)/3 */
#define C0       0.270598050073098492199861602684 /* 1 / (2 * sqrt(2 + sqrt(2))) */
#define C1       0.923879532511286756128183189400 /* (1 + sqrt(2)) /
                                                     (sqrt(2) * sqrt(2 + sqrt(2))) */

#define NTEST_POINTS 50 /* number of entries in test_points */
#define CENTERS      18 /* Index of first HTM triangle center in test_points */


static const struct htm_v3p test_points[NTEST_POINTS] = {
    { {       1.0,       0.0,       0.0 }, 0 }, /*  x */
    { {       0.0,       1.0,       0.0 }, 0 }, /*  y */
    { {       0.0,       0.0,       1.0 }, 0 }, /*  z */
    { {      -1.0,       0.0,       0.0 }, 0 }, /* -x */
    { {       0.0,      -1.0,       0.0 }, 0 }, /* -y */
    { {       0.0,       0.0,      -1.0 }, 0 }, /* -z */
    { {  SQRT_2_2,  SQRT_2_2,       0.0 }, 0 }, /* midpoint of  x and  y */
    { { -SQRT_2_2,  SQRT_2_2,       0.0 }, 0 }, /* midpoint of  y and -x */
    { { -SQRT_2_2, -SQRT_2_2,       0.0 }, 0 }, /* midpoint of -x and -y */
    { {  SQRT_2_2, -SQRT_2_2,       0.0 }, 0 }, /* midpoint of -y and  x */
    { {  SQRT_2_2,       0.0,  SQRT_2_2 }, 0 }, /* midpoint of  x and  z */
    { {       0.0,  SQRT_2_2,  SQRT_2_2 }, 0 }, /* midpoint of  y and  z */
    { { -SQRT_2_2,       0.0,  SQRT_2_2 }, 0 }, /* midpoint of -x and  z */
    { {       0.0, -SQRT_2_2,  SQRT_2_2 }, 0 }, /* midpoint of -y and  z */
    { {  SQRT_2_2,       0.0, -SQRT_2_2 }, 0 }, /* midpoint of  x and -z */
    { {       0.0,  SQRT_2_2, -SQRT_2_2 }, 0 }, /* midpoint of  y and -z */
    { { -SQRT_2_2,       0.0, -SQRT_2_2 }, 0 }, /* midpoint of -x and -z */
    { {       0.0, -SQRT_2_2, -SQRT_2_2 }, 0 }, /* midpoint of -y and -z */
    { {  SQRT_3_3,  SQRT_3_3,  SQRT_3_3 }, 0 }, /* center of N3 */
    { { -SQRT_3_3,  SQRT_3_3,  SQRT_3_3 }, 0 }, /* center of N2 */
    { { -SQRT_3_3, -SQRT_3_3,  SQRT_3_3 }, 0 }, /* center of N1 */
    { {  SQRT_3_3, -SQRT_3_3,  SQRT_3_3 }, 0 }, /* center of N0 */
    { {  SQRT_3_3,  SQRT_3_3, -SQRT_3_3 }, 0 }, /* center of S0 */
    { { -SQRT_3_3,  SQRT_3_3, -SQRT_3_3 }, 0 }, /* center of S1 */
    { { -SQRT_3_3, -SQRT_3_3, -SQRT_3_3 }, 0 }, /* center of S2 */
    { {  SQRT_3_3, -SQRT_3_3, -SQRT_3_3 }, 0 }, /* center of S3 */
    { {        C0,        C0,        C1 }, 0 }, /* center of N31 */
    { {        C1,        C0,        C0 }, 0 }, /* center of N32 */
    { {        C0,        C1,        C0 }, 0 }, /* center of N30 */
    { {       -C0,        C0,        C1 }, 0 }, /* center of N21 */
    { {       -C0,        C1,        C0 }, 0 }, /* center of N22 */
    { {       -C1,        C0,        C0 }, 0 }, /* center of N20 */
    { {       -C0,       -C0,        C1 }, 0 }, /* center of N11 */
    { {       -C1,       -C0,        C0 }, 0 }, /* center of N12 */
    { {       -C0,       -C1,        C0 }, 0 }, /* center of N10 */
    { {        C0,       -C0,        C1 }, 0 }, /* center of N01 */
    { {        C0,       -C1,        C0 }, 0 }, /* center of N02 */
    { {        C1,       -C0,        C0 }, 0 }, /* center of N00 */
    { {        C0,        C0,       -C1 }, 0 }, /* center of S01 */
    { {        C1,        C0,       -C0 }, 0 }, /* center of S00 */
    { {        C0,        C1,       -C0 }, 0 }, /* center of S02 */
    { {       -C0,        C0,       -C1 }, 0 }, /* center of S11 */
    { {       -C0,        C1,       -C0 }, 0 }, /* center of S10 */
    { {       -C1,        C0,       -C0 }, 0 }, /* center of S12 */
    { {       -C0,       -C0,       -C1 }, 0 }, /* center of S21 */
    { {       -C1,       -C0,       -C0 }, 0 }, /* center of S20 */
    { {       -C0,       -C1,       -C0 }, 0 }, /* center of S22 */
    { {        C0,       -C0,       -C1 }, 0 }, /* center of S31 */
    { {        C0,       -C1,       -C0 }, 0 }, /* center of S30 */
    { {        C1,       -C0,       -C0 }, 0 }, /* center of S32 */
};

/* Returns 1 if v (assumed to come from test_points) is a midpoint of
   two axis vectors (+/- x,y,z). These midpoints can legitimately end
   up in any of 3 children of the root triangles due to numerical
   inaccuracies. The actual child may be different depending on
   the compiler and optimization level being used. */
static int is_midpoint(const struct htm_v3 *v) {
   int i0 = v->x == 0.0;
   int i1 = v->y == 0.0;
   int i2 = v->z == 0.0;
   return i0 + i1 + i2 == 1; 
}

enum {
    S0 = (HTM_S0 + 8), S00 = (HTM_S0 + 8)*4, S01, S02, S03,
    S1 = (HTM_S1 + 8), S10 = (HTM_S1 + 8)*4, S11, S12, S13,
    S2 = (HTM_S2 + 8), S20 = (HTM_S2 + 8)*4, S21, S22, S23,
    S3 = (HTM_S3 + 8), S30 = (HTM_S3 + 8)*4, S31, S32, S33,
    N0 = (HTM_N0 + 8), N00 = (HTM_N0 + 8)*4, N01, N02, N03,
    N1 = (HTM_N1 + 8), N10 = (HTM_N1 + 8)*4, N11, N12, N13,
    N2 = (HTM_N2 + 8), N20 = (HTM_N2 + 8)*4, N21, N22, N23,
    N3 = (HTM_N3 + 8), N30 = (HTM_N3 + 8)*4, N31, N32, N33
};

struct test_results {
    int64_t id;
    int nranges;
    struct htm_range range[4];
};

static const struct test_results level0_results[NTEST_POINTS] = {
    { /*  x */  N3, 3, { {S0,S0} , {S3,N0} , {N3,N3} , {0,0} } },
    { /*  y */  N2, 2, { {S0,S1} , {N2,N3} , { 0, 0} , {0,0} } },
    { /*  z */  N3, 1, { {N0,N3} , { 0, 0} , { 0, 0} , {0,0} } },
    { /* -x */  N1, 2, { {S1,S2} , {N1,N2} , { 0, 0} , {0,0} } },
    { /* -y */  N0, 1, { {S2,N1} , { 0, 0} , { 0, 0} , {0,0} } },
    { /* -z */  S0, 1, { {S0,S3} , { 0, 0} , { 0, 0} , {0,0} } },
    { /* midpoint of  x and  y */ N3, 2, { {S0,S0} , {N3,N3} , {0,0} , {0,0} } },
    { /* midpoint of  y and -x */ N2, 2, { {S1,S1} , {N2,N2} , {0,0} , {0,0} } },
    { /* midpoint of -x and -y */ N1, 2, { {S2,S2} , {N1,N1} , {0,0} , {0,0} } },
    { /* midpoint of -y and  x */ N0, 1, { {S3,N0} , { 0, 0} , {0,0} , {0,0} } },
    { /* midpoint of  x and  z */ N3, 2, { {N0,N0} , {N3,N3} , {0,0} , {0,0} } },
    { /* midpoint of  y and  z */ N2, 1, { {N2,N3} , { 0, 0} , {0,0} , {0,0} } },
    { /* midpoint of -x and  z */ N1, 1, { {N1,N2} , { 0, 0} , {0,0} , {0,0} } },
    { /* midpoint of -y and  z */ N0, 1, { {N0,N1} , { 0, 0} , {0,0} , {0,0} } },
    { /* midpoint of  x and -z */ S0, 2, { {S0,S0} , {S3,S3} , {0,0} , {0,0} } },
    { /* midpoint of  y and -z */ S1, 1, { {S0,S1} , { 0, 0} , {0,0} , {0,0} } },
    { /* midpoint of -x and -z */ S2, 1, { {S1,S2} , { 0, 0} , {0,0} , {0,0} } },
    { /* midpoint of -y and -z */ S3, 1, { {S2,S3} , { 0, 0} , {0,0} , {0,0} } },
    { /* center of N3  */ N3, 1, { {N3,N3}, {0,0}, {0,0}, {0,0} } },
    { /* center of N2  */ N2, 1, { {N2,N2}, {0,0}, {0,0}, {0,0} } },
    { /* center of N1  */ N1, 1, { {N1,N1}, {0,0}, {0,0}, {0,0} } },
    { /* center of N0  */ N0, 1, { {N0,N0}, {0,0}, {0,0}, {0,0} } },
    { /* center of S0  */ S0, 1, { {S0,S0}, {0,0}, {0,0}, {0,0} } },
    { /* center of S1  */ S1, 1, { {S1,S1}, {0,0}, {0,0}, {0,0} } },
    { /* center of S2  */ S2, 1, { {S2,S2}, {0,0}, {0,0}, {0,0} } },
    { /* center of S3  */ S3, 1, { {S3,S3}, {0,0}, {0,0}, {0,0} } },
    { /* center of N31 */ N3, 1, { {N3,N3}, {0,0}, {0,0}, {0,0} } },
    { /* center of N32 */ N3, 1, { {N3,N3}, {0,0}, {0,0}, {0,0} } },
    { /* center of N30 */ N3, 1, { {N3,N3}, {0,0}, {0,0}, {0,0} } },
    { /* center of N21 */ N2, 1, { {N2,N2}, {0,0}, {0,0}, {0,0} } },
    { /* center of N22 */ N2, 1, { {N2,N2}, {0,0}, {0,0}, {0,0} } },
    { /* center of N20 */ N2, 1, { {N2,N2}, {0,0}, {0,0}, {0,0} } },
    { /* center of N11 */ N1, 1, { {N1,N1}, {0,0}, {0,0}, {0,0} } },
    { /* center of N12 */ N1, 1, { {N1,N1}, {0,0}, {0,0}, {0,0} } },
    { /* center of N10 */ N1, 1, { {N1,N1}, {0,0}, {0,0}, {0,0} } },
    { /* center of N01 */ N0, 1, { {N0,N0}, {0,0}, {0,0}, {0,0} } },
    { /* center of N02 */ N0, 1, { {N0,N0}, {0,0}, {0,0}, {0,0} } },
    { /* center of N00 */ N0, 1, { {N0,N0}, {0,0}, {0,0}, {0,0} } },
    { /* center of S01 */ S0, 1, { {S0,S0}, {0,0}, {0,0}, {0,0} } },
    { /* center of S00 */ S0, 1, { {S0,S0}, {0,0}, {0,0}, {0,0} } },
    { /* center of S02 */ S0, 1, { {S0,S0}, {0,0}, {0,0}, {0,0} } },
    { /* center of S11 */ S1, 1, { {S1,S1}, {0,0}, {0,0}, {0,0} } },
    { /* center of S10 */ S1, 1, { {S1,S1}, {0,0}, {0,0}, {0,0} } },
    { /* center of S12 */ S1, 1, { {S1,S1}, {0,0}, {0,0}, {0,0} } },
    { /* center of S21 */ S2, 1, { {S2,S2}, {0,0}, {0,0}, {0,0} } },
    { /* center of S20 */ S2, 1, { {S2,S2}, {0,0}, {0,0}, {0,0} } },
    { /* center of S22 */ S2, 1, { {S2,S2}, {0,0}, {0,0}, {0,0} } },
    { /* center of S31 */ S3, 1, { {S3,S3}, {0,0}, {0,0}, {0,0} } },
    { /* center of S30 */ S3, 1, { {S3,S3}, {0,0}, {0,0}, {0,0} } },
    { /* center of S32 */ S3, 1, { {S3,S3}, {0,0}, {0,0}, {0,0} } }
};

static const struct test_results level1_results[NTEST_POINTS] = {
    { /*  x */  N32, 4, { {S00,S00}, {S32,S32}, {N00,N00}, {N32,N32} } },
    { /*  y */  N22, 4, { {S02,S02}, {S10,S10}, {N22,N22}, {N30,N30} } },
    { /*  z */  N31, 4, { {N01,N01}, {N11,N11}, {N21,N21}, {N31,N31} } },
    { /* -x */  N12, 4, { {S12,S12}, {S20,S20}, {N12,N12}, {N20,N20} } },
    { /* -y */  N02, 4, { {S22,S22}, {S30,S30}, {N02,N02}, {N10,N10} } },
    { /* -z */  S01, 4, { {S01,S01}, {S11,S11}, {S21,S21}, {S31,S31} } },
    { /* midpoint of  x and  y */ 0, 4, { {S00,S00}, {S02,S03}, {N30,N30}, {N32,N33} } },
    { /* midpoint of  y and -x */ 0, 4, { {S10,S10}, {S12,S13}, {N20,N20}, {N22,N23} } },
    { /* midpoint of -x and -y */ 0, 4, { {S20,S20}, {S22,S23}, {N10,N10}, {N12,N13} } },
    { /* midpoint of -y and  x */ 0, 3, { {S30,S30}, {S32,N00}, {N02,N03}, {  0,  0} } },
    { /* midpoint of  x and  z */ 0, 3, { {N00,N01}, {N03,N03}, {N31,N33}, {  0,  0} } },
    { /* midpoint of  y and  z */ 0, 2, { {N21,N31}, {N33,N33}, {  0,  0}, {  0,  0} } },
    { /* midpoint of -x and  z */ 0, 2, { {N11,N21}, {N23,N23}, {  0,  0}, {  0,  0} } },
    { /* midpoint of -y and  z */ 0, 2, { {N01,N11}, {N13,N13}, {  0,  0}, {  0,  0} } },
    { /* midpoint of  x and -z */ 0, 3, { {S00,S01}, {S03,S03}, {S31,S33}, {  0,  0} } },
    { /* midpoint of  y and -z */ 0, 2, { {S01,S11}, {S13,S13}, {  0,  0}, {  0,  0} } },
    { /* midpoint of -x and -z */ 0, 2, { {S11,S21}, {S23,S23}, {  0,  0}, {  0,  0} } },
    { /* midpoint of -y and -z */ 0, 2, { {S21,S31}, {S33,S33}, {  0,  0}, {  0,  0} } },
    { /* center of N3  */ N33, 1, { {N33,N33}, {0,0}, {0,0}, {0,0} } },
    { /* center of N2  */ N23, 1, { {N23,N23}, {0,0}, {0,0}, {0,0} } },
    { /* center of N1  */ N13, 1, { {N13,N13}, {0,0}, {0,0}, {0,0} } },
    { /* center of N0  */ N03, 1, { {N03,N03}, {0,0}, {0,0}, {0,0} } },
    { /* center of S0  */ S03, 1, { {S03,S03}, {0,0}, {0,0}, {0,0} } },
    { /* center of S1  */ S13, 1, { {S13,S13}, {0,0}, {0,0}, {0,0} } },
    { /* center of S2  */ S23, 1, { {S23,S23}, {0,0}, {0,0}, {0,0} } },
    { /* center of S3  */ S33, 1, { {S33,S33}, {0,0}, {0,0}, {0,0} } },
    { /* center of N31 */ N31, 1, { {N31,N31}, {0,0}, {0,0}, {0,0} } },
    { /* center of N32 */ N32, 1, { {N32,N32}, {0,0}, {0,0}, {0,0} } },
    { /* center of N30 */ N30, 1, { {N30,N30}, {0,0}, {0,0}, {0,0} } },
    { /* center of N21 */ N21, 1, { {N21,N21}, {0,0}, {0,0}, {0,0} } },
    { /* center of N22 */ N22, 1, { {N22,N22}, {0,0}, {0,0}, {0,0} } },
    { /* center of N20 */ N20, 1, { {N20,N20}, {0,0}, {0,0}, {0,0} } },
    { /* center of N11 */ N11, 1, { {N11,N11}, {0,0}, {0,0}, {0,0} } },
    { /* center of N12 */ N12, 1, { {N12,N12}, {0,0}, {0,0}, {0,0} } },
    { /* center of N10 */ N10, 1, { {N10,N10}, {0,0}, {0,0}, {0,0} } },
    { /* center of N01 */ N01, 1, { {N01,N01}, {0,0}, {0,0}, {0,0} } },
    { /* center of N02 */ N02, 1, { {N02,N02}, {0,0}, {0,0}, {0,0} } },
    { /* center of N00 */ N00, 1, { {N00,N00}, {0,0}, {0,0}, {0,0} } },
    { /* center of S01 */ S01, 1, { {S01,S01}, {0,0}, {0,0}, {0,0} } },
    { /* center of S00 */ S00, 1, { {S00,S00}, {0,0}, {0,0}, {0,0} } },
    { /* center of S02 */ S02, 1, { {S02,S02}, {0,0}, {0,0}, {0,0} } },
    { /* center of S11 */ S11, 1, { {S11,S11}, {0,0}, {0,0}, {0,0} } },
    { /* center of S10 */ S10, 1, { {S10,S10}, {0,0}, {0,0}, {0,0} } },
    { /* center of S12 */ S12, 1, { {S12,S12}, {0,0}, {0,0}, {0,0} } },
    { /* center of S21 */ S21, 1, { {S21,S21}, {0,0}, {0,0}, {0,0} } },
    { /* center of S20 */ S20, 1, { {S20,S20}, {0,0}, {0,0}, {0,0} } },
    { /* center of S22 */ S22, 1, { {S22,S22}, {0,0}, {0,0}, {0,0} } },
    { /* center of S31 */ S31, 1, { {S31,S31}, {0,0}, {0,0}, {0,0} } },
    { /* center of S30 */ S30, 1, { {S30,S30}, {0,0}, {0,0}, {0,0} } },
    { /* center of S32 */ S32, 1, { {S32,S32}, {0,0}, {0,0}, {0,0} } }
};

static const struct test_results * const results[2] = {
    level0_results, level1_results
};


/*  Tests HTM indexing routines on predefined points.
 */
static void test_predefined_points()
{
    struct htm_v3p pts[NTEST_POINTS];
    int64_t ids[NTEST_POINTS];
    size_t i, j, k;
    int ret;
    int level;

    memset(pts, 0, sizeof(pts));
    memset(ids, 0, sizeof(ids));

    /* Failure tests */
    HTM_ASSERT(htm_v3_id(NULL, 0) == 0,
               "htm_v3_id() should have failed");
    HTM_ASSERT(htm_v3_id(&pts[0].v, -1) == 0,
               "htm_v3_id() should have failed");
    HTM_ASSERT(htm_v3_id(&pts[0].v, HTM_MAX_LEVEL + 1) == 0,
               "htm_v3_id() should have failed");
    HTM_ASSERT(htm_v3p_idsort(NULL, ids, 1, 0) != HTM_OK,
               "htm_v3p_idsort() should have failed");
    HTM_ASSERT(htm_v3p_idsort(pts, NULL, 1, 0) != HTM_OK,
               "htm_v3p_idsort() should have failed");
    HTM_ASSERT(htm_v3p_idsort(pts, ids, 1, -1) != HTM_OK,
               "htm_v3p_idsort() should have failed");
    HTM_ASSERT(htm_v3p_idsort(pts, ids, 1, HTM_MAX_LEVEL + 1) != HTM_OK,
               "htm_v3p_idsort() should have failed");
    HTM_ASSERT(htm_v3p_idsort(pts, ids, 0, 0) != HTM_OK,
               "htm_v3p_idsort() should have failed");
    for (i = 0; i < 8; ++i) {
        HTM_ASSERT(htm_level((int64_t) i) < 0,
                   "htm_level() should have failed");
    }
    HTM_ASSERT(htm_level(UINT64_C(0x3fffffffffffffff)) < 0,
               "htm_level() should have failed");

    for (level = 0; level < 2; ++level) {
        memcpy(pts, test_points, sizeof(test_points));
        ret = htm_v3p_idsort(pts, ids, NTEST_POINTS, level);
        HTM_ASSERT(ret == HTM_OK, "htm_v3p_idsort() failed");
        for (i = 0; i < NTEST_POINTS; ++i) {
            int64_t id = htm_v3_id(&pts[i].v, level);
            HTM_ASSERT(id != 0, "htm_v3_id() failed");
            if (id != ids[i]) {
                HTM_ASSERT(level > 0 && is_midpoint(&pts[i].v),
                           "htm_v3p_idsort() does not agree "
                           "with htm_v3_id()");
            }
            HTM_ASSERT(htm_level(id) == level, "htm_level() failed");
        }
        for (i = 0; i < NTEST_POINTS; ++i) {
            int64_t eid = results[level][i].id;
            int64_t id = htm_v3_id(&test_points[i].v, level);
            HTM_ASSERT(eid == id || eid == 0, "htm_v3_id() "
                       "did not produce expected result");
        }
        /* Test htm_v3p_idsort() on arrays of varying length */
        for (i = 1; i < NTEST_POINTS; ++i) {
            for (j = 0; j <= NTEST_POINTS - i; ++j) {
                memcpy(pts, test_points + j, i * sizeof(struct htm_v3p));
                ret = htm_v3p_idsort(pts, ids, i, level);
                HTM_ASSERT(ret == HTM_OK, "htm_v3p_idsort() failed");
                for (k = 0; k < i; ++k) {
                    int64_t id = htm_v3_id(&pts[k].v, level);
                    HTM_ASSERT(id != 0, "htm_v3_id() failed");
                    if (id != ids[k]) {
                        HTM_ASSERT(level > 0 && is_midpoint(&pts[k].v),
                                   "htm_v3p_idsort() does not agree "
                                   "with htm_v3_id()");
                    }
                }
            }
        }
    }
    /* Tests at subdivision levels 2 to HTM_MAX_LEVEL */
    for (i = CENTERS; i < NTEST_POINTS; ++i) {
        ids[i] = level1_results[i].id;
    }
    for (level = 2; level <= HTM_MAX_LEVEL; ++level) {
        size_t n = (level < 8) ? NTEST_POINTS : CENTERS + 8;
        for (i = CENTERS; i < n; ++i) {
            int64_t id = htm_v3_id(&test_points[i].v, level);
            int64_t eid = ids[i] * 4 + 3;
            HTM_ASSERT(id == eid, "htm_v3_id() did not produce "
                       "expected result (L%d, pt %d)", level, (int)i);
            ids[i] = eid;
            HTM_ASSERT(htm_level(id) == level, "htm_level() failed");
        }
    }
}


/*  Tests that htm_v3_id and htm_v3p_idsort agree for large
    arrays of random points.
 */
static void test_random_points()
{
    struct htm_v3p *pts;
    int64_t *ids;
    size_t i;
    int level, ret;
    const size_t n = 10000;

    pts = static_cast<htm_v3p *>(malloc(sizeof(struct htm_v3p) * n));
    HTM_ASSERT(pts != NULL, "memory allocation failed");
    ids = static_cast<int64_t*>(malloc(sizeof(int64_t) * n));
    HTM_ASSERT(ids != NULL, "memory allocation failed");

    for (level = 0; level <= HTM_MAX_LEVEL; ++level) {
        for (i = 0; i < n; ++i) {
            pts[i].v.x = htm_rand() - 0.5;
            pts[i].v.y = htm_rand() - 0.5;
            pts[i].v.z = htm_rand() - 0.5;
            htm_v3_normalize(&pts[i].v, &pts[i].v);
        }
        ret = htm_v3p_idsort(pts, ids, n, level);
        HTM_ASSERT(ret == HTM_OK, "htm_v3p_idsort() failed");
        for (i = 0; i < n; ++i) {
            int64_t id = htm_v3_id(&pts[i].v, level);
            HTM_ASSERT(id == ids[i], "htm_v3_id() does not agree "
                       "with htm_v3p_idsort()");
            HTM_ASSERT(htm_level(id) == level, "htm_level() failed");
        }
    }
    free(ids);
    free(pts);
}


/*  Tests HTM indexing of spherical circles.
 */
static void test_circles()
{
    struct htm_ids *ids = NULL;
    const struct htm_v3 *v = &test_points[0].v;
    double radius = 10.0;
    int i, j, level;
    enum htm_errcode err;

    /* Failure tests */
    HTM_ASSERT(htm_s2circle_ids(NULL, NULL, 0.0, 0, SIZE_MAX, &err) == NULL,
               "htm_s2circle_ids() should have failed");
    HTM_ASSERT(htm_s2circle_ids(NULL, v, 0.0, -1, SIZE_MAX, &err) == NULL,
               "htm_s2circle_ids() should have failed");
    HTM_ASSERT(htm_s2circle_ids(NULL, v, 0.0, HTM_MAX_LEVEL + 1, SIZE_MAX, &err) == NULL,
               "htm_s2circle_ids() should have failed");

    /* Test empty circle and all-sky constraint */
    ids = htm_s2circle_ids(NULL, v, -1.0, 0, SIZE_MAX, &err);
    HTM_ASSERT(ids != NULL && err == HTM_OK, "htm_s2circle_ids() failed");
    HTM_ASSERT(ids->n == 0, "htm_s2circle_ids() failed");
    ids = htm_s2circle_ids(ids, v, 180.0, 1, SIZE_MAX, &err);
    HTM_ASSERT(ids != NULL && err == HTM_OK, "htm_s2circle_ids() failed");
    HTM_ASSERT(ids->n == 1, "htm_s2circle_ids() failed");
    HTM_ASSERT(ids->range[0].min == 32 && ids->range[0].max == 63,
               "htm_s2circle_ids() failed");

    for (level = 0; level < 2; ++level) {
        for (i = 0; i < NTEST_POINTS; ++i) {
            int nr = results[level][i].nranges;
            ids = htm_s2circle_ids(ids, &test_points[i].v, radius, level, SIZE_MAX, &err);
            HTM_ASSERT(ids != NULL, "htm_s2circle_ids() failed");
            HTM_ASSERT(ids->n == (size_t) nr,
                       "htm_s2circle_ids() did not return the "
                       "expected number of ranges");
            for (j = 0; j < nr; ++j) {
                HTM_ASSERT(results[level][i].range[j].min == ids->range[j].min &&
                           results[level][i].range[j].max == ids->range[j].max,
                           "htm_s2circle_ids() did not return the expected ranges");
            }
        }
    }
    /* Tests at subdivision levels 2 to HTM_MAX_LEVEL */
    radius = 1.0;
    for (level = 2; level < 8; ++level, radius *= 0.5) {
        int n = (level < 8) ? NTEST_POINTS : CENTERS + 8;
        for (i = CENTERS; i < n; ++i) {
            int64_t id = htm_v3_id(&test_points[i].v, level);
            HTM_ASSERT(id != 0, "htm_v3_id() failed");
            ids = htm_s2circle_ids(ids, &test_points[i].v, radius, level, SIZE_MAX, &err);
            HTM_ASSERT(ids != NULL, "htm_s2circle_ids() failed");
            HTM_ASSERT(ids->n == 1,
                       "htm_s2circle_ids() did not return the "
                       "expected number of ranges");
            HTM_ASSERT(id == ids->range[0].min && id == ids->range[0].max,
                       "htm_s2circle_ids() did not return the expected ranges");
        }
    }
    free(ids);
}


/*  Tests HTM indexing of spherical convex polygons.
 */
static void test_polygons()
{
    struct htm_v3 sliver[3];
    struct htm_sc p;
    struct htm_s2cpoly *poly = NULL;
    struct htm_ids *ids = NULL;
    double radius = 10.0;
    int i, j, level;
    enum htm_errcode err;

    /* Failure tests */
    HTM_ASSERT(htm_s2cpoly_ids(NULL, NULL, 0, SIZE_MAX, &err) == NULL,
               "htm_s2cpoly_ids() should have failed");
    poly = htm_s2cpoly_line(&test_points[18].v, &test_points[23].v, 5.0, &err);
    HTM_ASSERT(poly != NULL, "htm_s2cpoly_line() failed");
    HTM_ASSERT(htm_s2cpoly_ids(NULL, poly, -1, SIZE_MAX, &err) == NULL,
               "htm_s2cpoly_ids() should have failed");
    HTM_ASSERT(htm_s2cpoly_ids(NULL, poly, HTM_MAX_LEVEL + 1, SIZE_MAX, &err) == NULL,
               "htm_s2cpoly_ids() should have failed");
    free(poly);
    /* test with lots of vertices */
    poly = htm_s2cpoly_ngon(&test_points[18].v, 1.0, 1000, &err);
    HTM_ASSERT(poly != NULL, "htm_s2cpoly_ngon() failed");
    ids = htm_s2cpoly_ids(ids, poly, 1, SIZE_MAX, &err);
    HTM_ASSERT(ids != NULL && err == HTM_OK, "htm_s2cpoly_ids() failed");
    HTM_ASSERT(ids->n == 1, "htm_s2cpoly_ids() failed");
    HTM_ASSERT(ids->range[0].min == N33 && ids->range[0].max == N33,
               "htm_s2cpoly_ids() failed");
    free(poly);

    for (level = 0; level < 2; ++level) {
        for (i = 0; i < NTEST_POINTS; ++i) {
            int nr = results[level][i].nranges;
            poly = htm_s2cpoly_ngon(&test_points[i].v, radius, 4, &err);
            HTM_ASSERT(poly != NULL, "htm_s2cpoly_ngon() failed");
            ids = htm_s2cpoly_ids(ids, poly, level, SIZE_MAX, &err);
            HTM_ASSERT(ids != NULL, "htm_s2cpoly_ids() failed");
            HTM_ASSERT(ids->n == (size_t) nr,
                       "htm_s2cpoly_ids() did not return the "
                       "expected number of ranges");
            for (j = 0; j < nr; ++j) {
                HTM_ASSERT(results[level][i].range[j].min == ids->range[j].min &&
                           results[level][i].range[j].max == ids->range[j].max,
                           "htm_s2cpoly_ids() did not return the expected ranges");
            }
            free(poly);
        }
    }
    /* Tests at subdivision levels 2 to HTM_MAX_LEVEL */
    radius = 1.0;
    for (level = 2; level < 8; ++level, radius *= 0.5) {
        int n = (level < 8) ? NTEST_POINTS : CENTERS + 8;
        for (i = CENTERS; i < n; ++i) {
            int64_t id = htm_v3_id(&test_points[i].v, level);
            HTM_ASSERT(id != 0, "htm_v3_id() failed");
            poly = htm_s2cpoly_ngon(&test_points[i].v, radius, 4, &err);
            HTM_ASSERT(poly != NULL, "htm_s2cpoly_ngon() failed");
            ids = htm_s2cpoly_ids(ids, poly, level, SIZE_MAX, &err);
            HTM_ASSERT(ids != NULL, "htm_s2cpoly_ids() failed");
            HTM_ASSERT(ids->n == 1,
                       "htm_s2cpoly_ids() did not return the "
                       "expected number of ranges");
            HTM_ASSERT(id == ids->range[0].min && id == ids->range[0].max,
                       "htm_s2cpoly_ids() did not return the expected ranges");
            free(poly);
        }
    }
    htm_sc_init(&p, 1.0, -1.0);
    htm_sc_tov3(&sliver[0], &p);
    htm_sc_init(&p, 359.0, 4.0);
    htm_sc_tov3(&sliver[1], &p);
    htm_sc_init(&p, 358.0, 3.0);
    htm_sc_tov3(&sliver[2], &p);
    poly = htm_s2cpoly_init(sliver, 3, &err);
    HTM_ASSERT(poly != NULL, "htm_s2cpoly_init() failed");
    ids = htm_s2cpoly_ids(ids, poly, 0, SIZE_MAX, &err);
    HTM_ASSERT(ids != NULL, "htm_s2cpoly_ids() failed");
    HTM_ASSERT(ids->n == 3,
               "htm_s2cpoly_ids() did not return the "
               "expected number of ranges");
    HTM_ASSERT(ids->range[0].min == S0 && ids->range[0].max == S0 &&
               ids->range[1].min == N0 && ids->range[1].max == N0 &&
               ids->range[2].min == N3 && ids->range[2].max == N3,
               "htm_s2cpoly_ids() did not return the "
               "expected ranges");
    ids = htm_s2cpoly_ids(ids, poly, 1, SIZE_MAX, &err);
    HTM_ASSERT(ids != NULL, "htm_s2cpoly_ids() failed");
    HTM_ASSERT(ids->n == 3,
               "htm_s2cpoly_ids() did not return the "
               "expected number of ranges");
    HTM_ASSERT(ids->range[0].min == S00 && ids->range[0].max == S00 &&
               ids->range[1].min == N00 && ids->range[1].max == N00 &&
               ids->range[2].min == N32 && ids->range[2].max == N32,
               "htm_s2cpoly_ids() did not return the "
               "expected ranges");
    free(ids);
    free(poly);
}


/*  Checks that ID range list a is a subset of b.
 */
static void check_subset(const struct htm_ids *a, const struct htm_ids *b)
{
    size_t i, j;
    for (i = 0, j = 0; i < a->n && j < b->n;) {
        if (a->range[i].min > b->range[j].max) {
            ++j;
            continue;
        }
        HTM_ASSERT(a->range[i].min >= b->range[j].min,
                   "fine range list is not a subset of coarse range list");
        HTM_ASSERT(a->range[i].max <= b->range[j].max,
                   "fine range list is not a subset of coarse range list");
        ++i;
    }
    HTM_ASSERT(i == a->n,
               "fine range list is not a subset of coarse range list");
    HTM_ASSERT(j == b->n - 1,
               "coarse range list includes unnecessary ranges");
}


/*  Tests adaptive coarsening of effective subdivision level with circles.
 */
static void test_adaptive_circle()
{
    static const double radii[3] = { 0.001, 0.1, 10.0 };
    struct htm_v3 center = test_points[18].v;
    struct htm_ids *coarse = NULL;
    struct htm_ids *fine = NULL;
    int i, level;
    enum htm_errcode err;

    for (i = 0; i < 3; ++i) {
        for (level = 0; level <= HTM_MAX_LEVEL; ++level) {
            fine = htm_s2circle_ids(fine, &center, radii[i], level, SIZE_MAX, &err);
            HTM_ASSERT(fine != NULL, "htm_s2cpoly_ids() failed");
            coarse = htm_s2circle_ids(coarse, &center, radii[i], level, 16, &err);
            HTM_ASSERT(coarse != NULL, "htm_s2cpoly_ids() failed");
            check_subset(fine, coarse); 
       }
    }
    free(coarse);
    free(fine);
}


/*  Tests adaptive coarsening of effective subdivision level with polygons.
 */
static void test_adaptive_poly()
{
    static const double radii[3] = { 0.001, 0.1, 1.0 };
    struct htm_s2cpoly *poly;
    struct htm_v3 center = test_points[18].v;
    struct htm_ids *coarse = NULL;
    struct htm_ids *fine = NULL;
    int i, level;
    enum htm_errcode err;

    for (i = 0; i < 3; ++i) {
        poly = htm_s2cpoly_ngon(&center, radii[i], 4, &err);
        HTM_ASSERT(poly != NULL, "htm_s2cpoly_ngon() failed");
        for (level = 0; level <= HTM_MAX_LEVEL; ++level) {
            fine = htm_s2cpoly_ids(fine, poly, level, SIZE_MAX, &err);
            HTM_ASSERT(fine != NULL, "htm_s2cpoly_ids() failed");
            coarse = htm_s2cpoly_ids(coarse, poly, level, 16, &err);
            HTM_ASSERT(coarse != NULL, "htm_s2cpoly_ids() failed");
            check_subset(fine, coarse);
       }
       free(poly);
    }
    free(coarse);
    free(fine);
}


/*  Test binary to decimal ID conversion.
 */
static void test_decimal_ids()
{
    static const struct htm_sc p[8] = {
        {  0.66800487, -19.4421234  },
        {  0.652247488, 66.0991364  },
        { 91.0758057,  -10.0598831  },
        { 91.2861862,   30.3034782  },
        { 183.978119,   13.1510801  },
        { 183.996658,   -9.89422703 },
        { 272.548676,  -36.8724594  },
        { 272.219696,   20.0442085  }
    };
    static const int expected[8] = {
        100010310,
        231120032,
        110001001,
        222212321,
        212021112,
        120032331,
        130102302,
        202020023
    };
    size_t i;
    for (i = 0; i < 8; ++i) {
        struct htm_v3 v;
        int64_t id;
        htm_sc_tov3(&v, &p[i]);
        id = htm_idtodec(htm_v3_id(&v, 7));
        HTM_ASSERT(id == expected[i], "htm_idtodec() failed");
    }
    HTM_ASSERT(htm_idtodec(0) == 0, "htm_idtodec() should have failed");
    HTM_ASSERT(htm_idtodec(UINT64_C(0x1fffffffffffffff)) == 0,
               "htm_idtodec() should have failed");
}


/*  Test HTM trixel retrieval.
 */
static void test_tri()
{
    struct htm_tri tri;
    /* test failure modes */
    HTM_ASSERT(htm_tri_init(NULL, N33) != HTM_OK,
               "htm_tri_init() should have failed");
    HTM_ASSERT(htm_tri_init(&tri, 1) != HTM_OK,
               "htm_tri_init() failed to detect invalid HTM id");
    /* test N33 */
    HTM_ASSERT(htm_tri_init(&tri, N33) == HTM_OK, "htm_tri_init() failed");
    HTM_ASSERT(tri.id == N33, "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[0], &test_points[10].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[1], &test_points[6].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[2], &test_points[11].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.center, &test_points[18].v, DBL_EPSILON),
               "htm_tri_init() failed");
    /* test N30 */
    HTM_ASSERT(htm_tri_init(&tri, N30) == HTM_OK, "htm_tri_init() failed");
    HTM_ASSERT(tri.id == N30, "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[0], &test_points[1].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[1], &test_points[11].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[2], &test_points[6].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.center, &test_points[28].v, DBL_EPSILON),
               "htm_tri_init() failed");
    /* test N31 */
    HTM_ASSERT(htm_tri_init(&tri, N31) == HTM_OK, "htm_tri_init() failed");
    HTM_ASSERT(tri.id == N31, "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[0], &test_points[2].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[1], &test_points[10].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[2], &test_points[11].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.center, &test_points[26].v, DBL_EPSILON),
               "htm_tri_init() failed");
    /* test N32 */
    HTM_ASSERT(htm_tri_init(&tri, N32) == HTM_OK, "htm_tri_init() failed");
    HTM_ASSERT(tri.id == N32, "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[0], &test_points[0].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[1], &test_points[6].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.verts[2], &test_points[10].v, DBL_EPSILON),
               "htm_tri_init() failed");
    HTM_ASSERT(v3_almost_equal(&tri.center, &test_points[27].v, DBL_EPSILON),
               "htm_tri_init() failed");
}


int main(int argc HTM_UNUSED, char **argv HTM_UNUSED) {
    htm_seed(123456789UL);
    test_predefined_points();
    test_random_points();
    test_circles();
    test_polygons();
    test_adaptive_circle();
    test_adaptive_poly();
    test_decimal_ids();
    test_tri();
    return 0;
}

