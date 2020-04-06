/** \file
    \brief      Unit tests for spherical geometry

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyhtm/geometry.h"
#include "cmp.h"
#include "rand.h"


#define HTM_ASSERT(pred, ...) \
    do { \
        if (!(pred)) { \
            fprintf(stderr, "[%s:%d]  ", __FILE__, __LINE__); \
            fprintf(stderr, #pred " is false:    " __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            exit(1); \
        } \
    } while(0)


static const struct htm_v3 nil = {  0.0,  0.0,  0.0 };
static const struct htm_v3 x   = {  1.0,  0.0,  0.0 };
static const struct htm_v3 y   = {  0.0,  1.0,  0.0 };
static const struct htm_v3 z   = {  0.0,  0.0,  1.0 };
static const struct htm_v3 nx  = { -1.0,  0.0,  0.0 };
static const struct htm_v3 ny  = {  0.0, -1.0,  0.0 };
static const struct htm_v3 nz  = {  0.0,  0.0, -1.0 };
static const struct htm_v3 xyz = {  1.0,  1.0,  1.0 };

static const struct htm_sc scx   = {   0.0,   0.0 };
static const struct htm_sc scy   = {  90.0,   0.0 };
static const struct htm_sc scz   = {   0.0,  90.0 };
static const struct htm_sc scnx  = { 180.0,   0.0 };
static const struct htm_sc scny  = { 270.0,   0.0 };
static const struct htm_sc scnz  = {   0.0, -90.0 };

#define NCENTERS 32
#define NLENGTHS 15

static const struct htm_v3 centers[NCENTERS] = {
    {  1.0,  0.0,  0.0 }, { -1.0,  0.0,  0.0 }, {  0.0,  1.0,  0.0 },
    {  0.0, -1.0,  0.0 }, {  0.0,  0.0,  1.0 }, {  0.0,  0.0, -1.0 },
    {  1.0,  1.0,  0.0 }, {  1.0, -1.0,  0.0 },
    {  1.0,  0.0,  1.0 }, {  1.0,  0.0, -1.0 },
    { -1.0,  1.0,  0.0 }, { -1.0, -1.0,  0.0 },
    { -1.0,  0.0,  1.0 }, { -1.0,  0.0, -1.0 },
    {  0.0,  1.0,  1.0 }, {  0.0,  1.0, -1.0 },
    {  0.0, -1.0,  1.0 }, {  0.0, -1.0, -1.0 },
    {  1.0,  1.0,  1.0 }, {  1.0,  1.0, -1.0 },
    {  1.0, -1.0,  1.0 }, {  1.0, -1.0, -1.0 },
    { -1.0,  1.0,  1.0 }, { -1.0,  1.0, -1.0 },
    { -1.0, -1.0,  1.0 }, { -1.0, -1.0, -1.0 },
    {  1.0, 1.0e-7, 1.0e-7 },
    {  1.0e-7, 1.0, 1.0e-7 },
    {  1.0e-7, 1.0e-7, 1.0 },
    { -1.0, 1.0e-7, -1.0e-7 },
    { -0.5, 10.0, -3.0 },
    {  10.0, -5.0, -2.0 }
};

static const size_t lengths[NLENGTHS] = {
    3, 4, 5, 6, 7, 13, 16, 31, 32, 33, 64, 128, 129, 1024, 65536
};


/*  Test utility functions.
 */
static void test_utils()
{
    double angle;
    /* htm_angred */
    HTM_ASSERT(htm_angred(45.0) == 45.0, "htm_angred() failed");
    HTM_ASSERT(htm_angred(-180.0) == 180.0, "htm_angred() failed");
    HTM_ASSERT(htm_angred(540.0) == 180.0, "htm_angred() failed");
    HTM_ASSERT(htm_angred(HTM_DEG_IN_CIRCLE) == 0.0, "htm_angred() failed");
    HTM_ASSERT(htm_angred(-DBL_EPSILON) == 0.0, "htm_angred() failed");
    HTM_ASSERT(htm_angred(HTM_DEG_IN_CIRCLE - DBL_EPSILON) == 0.0, "htm_angred() failed");
    HTM_ASSERT(htm_angred(DBL_EPSILON) == DBL_EPSILON, "htm_angred() failed");

    /* htm_clamp */
    angle = -91.0;
    HTM_ASSERT(htm_clamp(angle, -90.0, 90.0) == -90.0, "htm_clamp() failed");
    angle = 91.0;
    HTM_ASSERT(htm_clamp(angle, -90.0, 90.0) == 90.0, "htm_clamp() failed");
    angle = 45.0;
    HTM_ASSERT(htm_clamp(angle, -90.0, 90.0) == angle, "htm_clamp() failed");    
}

/*  Test basic vector ops.
 */
static void test_v3_basics()
{
    struct htm_v3 v;

    /* htm_v3_add */
    htm_v3_add(&v, &x, &nx);
    HTM_ASSERT(v3_equal(&v, &nil), "htm_v3_add() failed");
    htm_v3_add(&v, &y, &ny);
    HTM_ASSERT(v3_equal(&v, &nil), "htm_v3_add() failed");
    htm_v3_add(&v, &z, &nz);
    HTM_ASSERT(v3_equal(&v, &nil), "htm_v3_add() failed");

    /* htm_v3_sub and htm_v3_mul */
    htm_v3_sub(&v, &x, &nx);
    htm_v3_mul(&v, &v, 0.5);
    htm_v3_neg(&v, &v);
    HTM_ASSERT(v3_equal(&v, &nx),
               "htm_v3_sub(), htm_v3_mul(), or htm_v3_neg() failed");
    htm_v3_sub(&v, &y, &ny);
    htm_v3_mul(&v, &v, 0.5);
    htm_v3_neg(&v, &v);
    HTM_ASSERT(v3_equal(&v, &ny),
               "htm_v3_sub(), htm_v3_mul(), or htm_v3_neg() failed");
    htm_v3_sub(&v, &z, &nz);
    htm_v3_mul(&v, &v, 0.5);
    htm_v3_neg(&v, &v);
    HTM_ASSERT(v3_equal(&v, &nz),
               "htm_v3_sub(), htm_v3_mul(), or htm_v3_neg() failed");

    /* htm_v3_norm2, htm_v3_norm, and htm_v3_normalize */
    v.x = 1.0; v.y = 2.0; v.z = -3.0;
    HTM_ASSERT(htm_v3_norm2(&v) == 14.0, "htm_v3_norm2() failed");
    HTM_ASSERT(almost_equal(htm_v3_norm(&v), sqrt(14.0), DBL_EPSILON),
               "htm_v3_norm2() failed");
    htm_v3_normalize(&v, &v);
    HTM_ASSERT(almost_equal(htm_v3_norm(&v), 1.0, 4.0*DBL_EPSILON),
               "htm_v3_normalize() failed");

    /* htm_v3_dot */
    HTM_ASSERT(htm_v3_dot(&x, &x) == 1.0, "htm_v3_dot() failed");
    HTM_ASSERT(htm_v3_dot(&y, &y) == 1.0, "htm_v3_dot() failed"); 
    HTM_ASSERT(htm_v3_dot(&z, &z) == 1.0, "htm_v3_dot() failed"); 
    HTM_ASSERT(htm_v3_dot(&x, &y) == 0.0, "htm_v3_dot() failed"); 
    HTM_ASSERT(htm_v3_dot(&x, &z) == 0.0, "htm_v3_dot() failed"); 
    HTM_ASSERT(htm_v3_dot(&y, &z) == 0.0, "htm_v3_dot() failed"); 

    /* htm_v3_cross */
    htm_v3_cross(&v, &x, &y);
    HTM_ASSERT(v3_equal(&v, &z), "htm_v3_cross() failed");
    htm_v3_cross(&v, &y, &x);
    HTM_ASSERT(v3_equal(&v, &nz), "htm_v3_cross() failed");
    htm_v3_cross(&v, &y, &z);
    HTM_ASSERT(v3_equal(&v, &x), "htm_v3_cross() failed");
    htm_v3_cross(&v, &z, &y);
    HTM_ASSERT(v3_equal(&v, &nx), "htm_v3_cross() failed");
    htm_v3_cross(&v, &z, &x);
    HTM_ASSERT(v3_equal(&v, &y), "htm_v3_cross() failed");
    htm_v3_cross(&v, &x, &z);
    HTM_ASSERT(v3_equal(&v, &ny), "htm_v3_cross() failed");

    /* htm_v3_rcross, htm_v3_mul, htm_v3_div */
    htm_v3_rcross(&v, &x, &y);
    htm_v3_mul(&v, &v, 0.5);
    HTM_ASSERT(v3_equal(&v, &z), "htm_v3_rcross() or htm_v3_mul() failed");
    htm_v3_rcross(&v, &y, &x);
    htm_v3_div(&v, &v, 2.0);
    HTM_ASSERT(v3_equal(&v, &nz), "htm_v3_rcross() or htm_v3_div() failed");
    htm_v3_rcross(&v, &y, &z);
    htm_v3_mul(&v, &v, 0.5);
    HTM_ASSERT(v3_equal(&v, &x), "htm_v3_rcross() or htm_v3_mul() failed");
    htm_v3_rcross(&v, &z, &y);
    htm_v3_div(&v, &v, 2.0);
    HTM_ASSERT(v3_equal(&v, &nx), "htm_v3_rcross() or htm_v3_div() failed");
    htm_v3_rcross(&v, &z, &x);
    htm_v3_mul(&v, &v, 0.5);
    HTM_ASSERT(v3_equal(&v, &y), "htm_v3_rcross() or htm_v3_mul() failed");
    htm_v3_rcross(&v, &x, &z);
    htm_v3_div(&v, &v, 2.0);
    HTM_ASSERT(v3_equal(&v, &ny), "htm_v3_rcross() or htm_v3_div() failed");

    /* htm_v3_centroid */
    HTM_ASSERT(htm_v3_centroid(NULL, &x, 1) != HTM_OK,
               "htm_v3_centroid() should have failed");
    HTM_ASSERT(htm_v3_centroid(&v, NULL, 1) != HTM_OK,
               "htm_v3_centroid() should have failed");
    HTM_ASSERT(htm_v3_centroid(&v, &x, 0) != HTM_OK,
               "htm_v3_centroid() should have failed");
    HTM_ASSERT(htm_v3_centroid(&v, &x, 1) == HTM_OK, "htm_v3_centroid() failed");
    HTM_ASSERT(v3_almost_equal(&v, &x, DBL_EPSILON), "htm_v3_centroid() failed");
}


/*  Test spherical coordinates and conversion to and from a 3-vector.
 */
static void test_sc()
{
    struct htm_v3 v1, v2;
    struct htm_sc p1, p2;

    HTM_ASSERT(htm_v3_tosc(NULL, &x) != HTM_OK,
               "htm_v3_tosc() should have failed");
    HTM_ASSERT(htm_v3_tosc(&p2, NULL) != HTM_OK,
               "htm_v3_tosc() should have failed");
    htm_v3_tosc(&p1, &x);
    htm_sc_tov3(&v1, &p1);
    HTM_ASSERT(htm_sc_tov3(NULL, &p1) != HTM_OK,
               "htm_sc_tov390 should have failed");
    HTM_ASSERT(htm_sc_tov3(&v2, NULL) != HTM_OK,
               "htm_sc_tov390 should have failed");
    HTM_ASSERT(sc_equal(&p1, &scx), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &x, DBL_EPSILON), "htm_sc_tov3() failed");
    htm_v3_tosc(&p1, &z);
    htm_sc_tov3(&v1, &p1);
    HTM_ASSERT(sc_almost_equal(&p1, &scz, DBL_EPSILON), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &z, DBL_EPSILON), "htm_sc_tov3() failed");
    htm_v3_tosc(&p1, &nz);
    htm_sc_tov3(&v1, &p1);
    HTM_ASSERT(sc_almost_equal(&p1, &scnz, DBL_EPSILON), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &nz, DBL_EPSILON), "htm_sc_tov3() failed");
    htm_v3_tosc(&p1, &y);
    htm_sc_tov3(&v1, &p1);
    HTM_ASSERT(sc_almost_equal(&p1, &scy, DBL_EPSILON), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &y, DBL_EPSILON), "htm_sc_tov3() failed");
    htm_v3_tosc(&p1, &nx);
    htm_sc_tov3(&v1, &p1);
    HTM_ASSERT(sc_almost_equal(&p1, &scnx, DBL_EPSILON), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &nx, DBL_EPSILON), "htm_sc_tov3() failed");
    htm_v3_tosc(&p1, &ny);
    htm_sc_tov3(&v1, &p1);
    HTM_ASSERT(sc_almost_equal(&p1, &scny, DBL_EPSILON), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &ny, DBL_EPSILON), "htm_sc_tov3() failed");

    p2.lon = 45.0; p2.lat = 35.2643896827547;
    v1.x = 1.0; v1.y = 1.0; v1.z = 1.0;
    htm_v3_tosc(&p1, &v1);
    htm_sc_tov3(&v2, &p1);
    htm_v3_normalize(&v1, &v1);
    HTM_ASSERT(sc_almost_equal(&p1, &p2, 8.0*DBL_EPSILON), "htm_v3_tosc() failed");
    HTM_ASSERT(v3_almost_equal(&v1, &v2, 2.0*DBL_EPSILON), "htm_sc_tov3() failed");
}

static void test_v3_ne()
{
    struct htm_v3 n, e, v;
    enum htm_errcode err;

    /* failure modes */
    HTM_ASSERT(htm_v3_ne(NULL, &e, &x) != HTM_OK,
               "htm_v3_ne() should have failed");
    HTM_ASSERT(htm_v3_ne(&n, NULL, &x) != HTM_OK,
               "htm_v3_ne() should have failed");
    HTM_ASSERT(htm_v3_ne(&n, &e, NULL) != HTM_OK,
               "htm_v3_ne() should have failed");
    HTM_ASSERT(htm_v3_ne(&n, &e, &nil) != HTM_OK,
               "htm_v3_ne() should have failed");
    /* test on x/y axes */
    err = htm_v3_ne(&n, &e, &x);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&n, &z, DBL_EPSILON), "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&e, &y, DBL_EPSILON), "htm_v3_ne() failed");
    err = htm_v3_ne(&n, &e, &y);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&n, &z, DBL_EPSILON), "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&e, &nx, DBL_EPSILON), "htm_v3_ne() failed");
    err = htm_v3_ne(&n, &e, &nx);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&n, &z, DBL_EPSILON), "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&e, &ny, DBL_EPSILON), "htm_v3_ne() failed");
    err = htm_v3_ne(&n, &e, &ny);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&n, &z, DBL_EPSILON), "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&e, &x, DBL_EPSILON), "htm_v3_ne() failed");
    /* test on root HTM triangle midpoints */
    v.x = 1.0; v.y = 0.0; v.z = 1.0;
    err = htm_v3_ne(&n, &e, &v);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(htm_v3_dot(&n, &z) > 0.0, "htm_v3_ne() failed");
    HTM_ASSERT(fabs(htm_v3_dot(&n, &e)) < 4.0*DBL_EPSILON, "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&e, &y, DBL_EPSILON), "htm_v3_ne() failed");
    v.x = 1.0; v.z = 0.0; v.z = -1.0;
    err = htm_v3_ne(&n, &e, &v);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(htm_v3_dot(&n, &z) > 0.0, "htm_v3_ne() failed");
    HTM_ASSERT(fabs(htm_v3_dot(&n, &e)) < 4.0*DBL_EPSILON, "htm_v3_ne() failed");
    HTM_ASSERT(v3_almost_equal(&e, &y, DBL_EPSILON), "htm_v3_ne() failed");
    v.x = v.y = v.z = -1.0;
    err = htm_v3_ne(&n, &e, &v);
    HTM_ASSERT(err == HTM_OK, "htm_v3_ne() failed");
    HTM_ASSERT(htm_v3_dot(&n, &z) > 0.0, "htm_v3_ne() failed");
    HTM_ASSERT(fabs(htm_v3_dot(&n, &e)) < 4.0*DBL_EPSILON, "htm_v3_ne() failed");
    v.x = 1.0; v.y = -1.0; v.z = 0.0;
    htm_v3_normalize(&v, &v);
    HTM_ASSERT(v3_almost_equal(&e, &v, 2.0*DBL_EPSILON), "htm_v3_ne() failed");
}

static void test_v3_rot()
{
    struct htm_v3 v, k;
    double angle;
    enum htm_errcode err;

    /* htm_v3_tanrot failure modes */
    HTM_ASSERT(htm_v3_tanrot(NULL, &x, &y, 1.0) != HTM_OK,
               "htm_v3_tanrot() should have failed");
    HTM_ASSERT(htm_v3_tanrot(&angle, NULL, &y, 1.0) != HTM_OK,
               "htm_v3_tanrot() should have failed");
    HTM_ASSERT(htm_v3_tanrot(&angle, &x, NULL, 1.0) != HTM_OK,
               "htm_v3_tanrot() should have failed");
    HTM_ASSERT(htm_v3_tanrot(&angle, &x, &y, 0.0) != HTM_OK,
               "htm_v3_tanrot() should have failed");
    HTM_ASSERT(htm_v3_tanrot(&angle, &x, &y, -1.0) != HTM_OK,
               "htm_v3_tanrot() should have failed");
    HTM_ASSERT(htm_v3_tanrot(&angle, &x, &x, 1.0) != HTM_OK,
               "htm_v3_tanrot() should have failed");

    /* htm_v3_rot failure modes */
    HTM_ASSERT(htm_v3_rot(NULL, &x, &y, 1.0) != HTM_OK,
               "htm_v3_rot() should have failed");
    HTM_ASSERT(htm_v3_rot(&v, NULL, &y, 1.0) != HTM_OK,
               "htm_v3_rot() should have failed");
    HTM_ASSERT(htm_v3_rot(&v, &x, NULL, 1.0) != HTM_OK,
               "htm_v3_rot() should have failed");
    HTM_ASSERT(htm_v3_rot(&v, &x, &nil, 1.0) != HTM_OK,
               "htm_v3_rot() should have failed");
    HTM_ASSERT(htm_v3_rot(&v, &x, &z, 0.0) == HTM_OK, "htm_v3_rot() failed");
    HTM_ASSERT(v3_almost_equal(&v, &x, DBL_EPSILON), "htm_v3_rot() failed");

    err = htm_v3_rot(&v, &x, &z, 90.0);
    HTM_ASSERT(err == HTM_OK, "htm_v3_rot() failed");
    HTM_ASSERT(v3_almost_equal(&v, &y, 2.0*DBL_EPSILON), "htm_v3_rot() failed");
    err = htm_v3_rot(&v, &x, &nz, 90.0);
    HTM_ASSERT(err == HTM_OK, "htm_v3_rot() failed");
    HTM_ASSERT(v3_almost_equal(&v, &ny, 2.0*DBL_EPSILON), "htm_v3_rot() failed");
    err = htm_v3_rot(&v, &x, &z, -90.0);
    HTM_ASSERT(err == HTM_OK, "htm_v3_rot() failed");
    HTM_ASSERT(v3_almost_equal(&v, &ny, 2.0*DBL_EPSILON), "htm_v3_rot() failed");
    k.x = k.y = k.z = 1.0;
    htm_v3_normalize(&k, &k);
    err = htm_v3_rot(&v, &y, &k, 120.0);
    HTM_ASSERT(err == HTM_OK, "htm_v3_rot() failed");
    HTM_ASSERT(v3_almost_equal(&v, &z, 2.0*DBL_EPSILON), "htm_v3_rot() failed");
    err = htm_v3_rot(&v, &z, &k, 120.0);
    HTM_ASSERT(err == HTM_OK, "htm_v3_rot() failed");
    HTM_ASSERT(v3_almost_equal(&v, &x, 2.0*DBL_EPSILON), "htm_v3_rot() failed");
}


static void test_ang_dist()
{
    static const struct htm_v3 points[12] = {
        { 1.0, 1.0, 0.0 }, { 1.0,  0.0,  0.0 },
        { 1.0, 1.0, 0.0 }, { 0.0,  1.0,  0.0 },
        { 1.0, 0.0, 1.0 }, { 1.0,  0.0,  0.0 },
        { 0.0, 1.0, 1.0 }, { 0.0,  0.0,  1.0 },
        { 1.0, 1.0, 0.0 }, { 1.0, -1.0,  0.0 },
        { 1.0, 0.0, 1.0 }, { 1.0,  0.0, -1.0 }
    };
    static const double angles[6] = { 45.0, 45.0, 45.0, 45.0, 90.0, 90.0 };
    static const double dist2s[6] = {
        0.58578643762690495119831127579, /* 2 - sqrt(2) */
        0.58578643762690495119831127579,
        0.58578643762690495119831127579,
        0.58578643762690495119831127579,
        2.0,
        2.0
    };
    struct htm_v3 v1, v2;
    struct htm_sc p1, p2;
    size_t i;

    /* htm_sc_dist2 */
    HTM_ASSERT(almost_equal(htm_sc_dist2(&scy, &scz), 2.0, 4.0*DBL_EPSILON),
               "htm_sc_dist2() failed");
    HTM_ASSERT(htm_sc_dist2(&scy, &scy) == 0.0, "htm_sc_dist2() failed");
    HTM_ASSERT(almost_equal(htm_sc_dist2(&scz, &scnz), 4.0, 4.0*DBL_EPSILON),
               "htm_sc_dist2() failed");

    /* htm_sc_angsep */
    HTM_ASSERT(almost_equal(htm_sc_angsep(&scx, &scnz), 90.0, 4.0*DBL_EPSILON),
               "htm_sc_angsep() failed");
    HTM_ASSERT(htm_sc_angsep(&scny, &scny) == 0.0, "htm_sc_angsep() failed");
    HTM_ASSERT(almost_equal(htm_sc_angsep(&scnx, &scx), 180.0, 4.0*DBL_EPSILON),
               "htm_sc_angsep() failed");

    /* htm_v3_dist2 */
    HTM_ASSERT(htm_v3_dist2(&x, &y) == 2.0, "htm_v3_dist2() failed");
    HTM_ASSERT(htm_v3_dist2(&x, &x) == 0.0, "htm_v3_dist2() failed");
    HTM_ASSERT(htm_v3_dist2(&x, &nx) == 4.0, "htm_v3_dist2() failed");

    /* htm_v3_angsepu */
    HTM_ASSERT(htm_v3_angsep(&x, &nil) == 0.0, "htm_v3_angsepu() failed");
    HTM_ASSERT(htm_v3_angsepu(&x, &x) == 0.0, "htm_v3_angsepu() failed");
    HTM_ASSERT(almost_equal(htm_v3_angsepu(&z, &y), 90.0, 2.0*DBL_EPSILON),
               "htm_v3_angsepu() failed");
    HTM_ASSERT(almost_equal(htm_v3_angsepu(&y, &ny), 180.0, 2.0*DBL_EPSILON), 
               "htm_v3_angsepu() failed");

    /* htm_v3_angsep */
    HTM_ASSERT(htm_v3_angsep(&x, &x) == 0.0, "htm_v3_angsep() failed");
    HTM_ASSERT(almost_equal(htm_v3_angsep(&z, &y), 90.0, 2.0*DBL_EPSILON),
               "htm_v3_angsep() failed");
    HTM_ASSERT(almost_equal(htm_v3_angsep(&y, &ny), 180.0, 2.0*DBL_EPSILON), 
               "htm_v3_angsep() failed");

    for (i = 0; i < 6; ++i) {
        double a1, a2, a3, angle = angles[i];
        double d1, d2, dist2 = dist2s[i];
        v1 = points[2*i];
        v2 = points[2*i + 1];
        htm_v3_tosc(&p1, &v1);
        htm_v3_tosc(&p2, &v2);
        a1 = htm_sc_angsep(&p1, &p2);
        d1 = htm_sc_dist2(&p1, &p2);
        a2 = htm_v3_angsep(&v1, &v2);
        htm_v3_normalize(&v1, &v1);
        htm_v3_normalize(&v2, &v2);
        a3 = htm_v3_angsepu(&v1, &v2);
        d2 = htm_v3_dist2(&v1, &v2);
        HTM_ASSERT(almost_equal(dist2, d1, 4.0*DBL_EPSILON), "htm_sc_dist2() failed");
        HTM_ASSERT(almost_equal(dist2, d2, 4.0*DBL_EPSILON), "htm_v3_dist2() failed");
        HTM_ASSERT(almost_equal(angle, a1, 4.0*DBL_EPSILON), "htm_sc_angsep() failed");
        HTM_ASSERT(almost_equal(angle, a2, 4.0*DBL_EPSILON), "htm_v3_angsep() failed");
        HTM_ASSERT(almost_equal(angle, a3, 4.0*DBL_EPSILON), "htm_v3_angsepu() failed");
    }
}


static void test_s2cpoly()
{
    static const double bad_angles[4] = { -1.0, 0.0, 90.0, 100.0 };

    enum htm_errcode err;
    struct htm_v3 points[3];
    struct htm_s2cpoly *poly;
    size_t i;

    /* test failure modes */
    points[0] = x; points[1] = y; points[2] = z;
    HTM_ASSERT(htm_s2cpoly_init(points, 1, &err) == NULL,
               "htm_s2cpoly_init() should have failed");
    HTM_ASSERT(htm_s2cpoly_init(points, 2, &err) == NULL,
               "htm_s2cpoly_init() should have failed");
    HTM_ASSERT(htm_s2cpoly_ngon(&x, 10.0, 1, &err) == NULL,
               "htm_s2cpoly_ngon() should have failed");
    HTM_ASSERT(htm_s2cpoly_ngon(&x, 10.0, 2, &err) == NULL,
               "htm_s2cpoly_ngon() should have failed");
    HTM_ASSERT(htm_s2cpoly_init(NULL, 3, &err) == NULL,
               "htm_s2cpoly_init() should have failed");
    HTM_ASSERT(htm_s2cpoly_box(NULL, 1.0, 1.0, 0.0, &err) == NULL,
               "htm_s2cpoly_box() should have failed");
    HTM_ASSERT(htm_s2cpoly_box(&nil, 1.0, 1.0, 0.0, &err) == NULL,
               "htm_s2cpoly_box() should have failed");
    HTM_ASSERT(htm_s2cpoly_ngon(NULL, 1.0, 10, &err) == NULL,
               "htm_s2cpoly_ngon() should have failed");
    HTM_ASSERT(htm_s2cpoly_ngon(&nil, 1.0, 10, &err) == NULL,
               "htm_s2cpoly_ngon() should have failed");
    HTM_ASSERT(htm_s2cpoly_clone(NULL) == NULL,
               "htm_s2cpoly_clone() should have failed");
    for (i = 0; i < 4; ++i) {
        double a = bad_angles[i];
        HTM_ASSERT(htm_s2cpoly_ngon(&x, a, 3, &err) == NULL,
                   "htm_s2cpoly_ngon() should have failed");
        HTM_ASSERT(htm_s2cpoly_box(&z, a, 10.0, 0.0, &err) == NULL,
                   "htm_s2cpoly_box() should have failed");
        HTM_ASSERT(htm_s2cpoly_box(&z, 10.0, a, 0.0, &err) == NULL,
                   "htm_s2cpoly_box() should have failed");
        HTM_ASSERT(htm_s2cpoly_box(&z, a, a, 0.0, &err) == NULL,
                   "htm_s2cpoly_box() should have failed");
        HTM_ASSERT(htm_s2cpoly_line(&x, &y, a, &err) == NULL,
                   "htm_s2cpoly_line() should have failed");
    } 

    /* test polygons and their area */
    HTM_ASSERT(htm_s2cpoly_area(NULL) == 0.0, "htm_s2cpoly_area() failed");
    poly = htm_s2cpoly_init(points, 3, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_init() failed");
    HTM_ASSERT(almost_equal(htm_s2cpoly_area(poly), 0.5 * M_PI, 8.0 * DBL_EPSILON),
               "htm_s2cpoly_area() failed");
    free(poly);
    points[2].x = points[2].y = points[2].z = 1.0;
    htm_v3_normalize(&points[2], &points[2]);
    poly = htm_s2cpoly_init(points, 3, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_init() failed");
    HTM_ASSERT(almost_equal(htm_s2cpoly_area(poly), M_PI / 6.0, 8.0 * DBL_EPSILON),
               "htm_s2cpoly_area() failed");
    free(poly);
    poly = htm_s2cpoly_ngon(&z, 90.0 - 0.1/HTM_ARCSEC_PER_DEG, 65536, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
    HTM_ASSERT(almost_equal(htm_s2cpoly_area(poly), 2.0 * M_PI, 1.0e-3),
               "htm_s2cpoly_area() failed");
    free(poly);
    poly = htm_s2cpoly_box(&x, 0.01, 0.01, 90.0, &err);
    HTM_ASSERT(almost_equal(htm_s2cpoly_area(poly),
                            0.0001 * HTM_RAD_PER_DEG * HTM_RAD_PER_DEG,
                            1.0e-6), "htm_s2cpoly_area() failed");
    free(poly);
}


/*  Randomly generates a list of n points in the hemisphere given by the
    plane normal n. Space for n + 1 points is allocated, so that an opposing
    point can be added to the end of the list.
 */
static struct htm_v3 * gen_hemis(const struct htm_v3 *v, size_t n)
{
    struct htm_v3 p, r, tmp, north, east;
    struct htm_v3 *points;
    double cx, cy, cz;
    size_t i;

    HTM_ASSERT(n != 0 && v != NULL, "illegal arguments to gen_hemis");
    points = static_cast<htm_v3 *>(malloc((n + 1) * sizeof(struct htm_v3)));
    HTM_ASSERT(points != NULL, "memory allocation failed");
    htm_v3_normalize(&p, v);
    htm_v3_ne(&north, &east, &p);
    for (i = 0; i < n; ++i) {
        do {
            cx = htm_rand();
            cy = htm_rand();
            cx = (1.0 - 2.0e-7)*cx - (0.5 - 1.0e-7);
            cy = (1.0 - 2.0e-7)*cy - (0.5 - 1.0e-7);
            cz = 1.0 - cx * cx - cy * cy;
        } while (cz < 0.0);
        cz = sqrt(cz);
        htm_v3_mul(&r, v, cz);
        htm_v3_mul(&tmp, &north, cx);
        htm_v3_add(&r, &r, &tmp);
        htm_v3_mul(&tmp, &east, cy);
        htm_v3_add(&r, &r, &tmp);
        htm_v3_normalize(&r, &r);
        points[i] = r;
    }
    return points;
}

/*  Adds a point p to points s.t. points becomes non-hemispherical.
 */
static void gen_opposing(struct htm_v3 *points, size_t n)
{
    struct htm_v3 v;
    if (n == 0) {
        return;
    }
    htm_v3_centroid(&v, points, (n > 3) ? 3 : n);
    htm_v3_normalize(&v, &v);
    htm_v3_neg(&points[n], &v);
}

/*  Generates points on circles of center v and -v with radius r.
 */
static struct htm_v3 * gen_circles(const struct htm_v3 *v, double r, size_t n)
{
   struct htm_v3 cen;
   struct htm_s2cpoly *poly;
   struct htm_v3 *points;
   enum htm_errcode err;

   points = (struct htm_v3 *) malloc(2 * n * sizeof(struct htm_v3));
   HTM_ASSERT(points != NULL, "memory allocation failed");
   htm_v3_normalize(&cen, v);
   poly = htm_s2cpoly_ngon(&cen, r, n, &err);
   HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
   memcpy(points, poly->ve, n * sizeof(struct htm_v3));
   free(poly);
   htm_v3_neg(&cen, &cen);
   poly = htm_s2cpoly_ngon(&cen, r, n, &err);
   HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
   memcpy(points + n, poly->ve, n * sizeof(struct htm_v3));
   free(poly);
   return points;
}


/*  Test polygon padding.
 */
static void test_pad()
{
    struct htm_v3 *points;
    struct htm_s2cpoly *poly, *clone;
    size_t i, j;
    enum htm_errcode err;

    points = gen_circles(&xyz, 10.0, 10);
    poly = htm_s2cpoly_init(points, 10, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_init() failed");
    clone = htm_s2cpoly_clone(poly);
    HTM_ASSERT(clone != NULL, "htm_s2cpoly_clone() failed");
    HTM_ASSERT(htm_s2cpoly_pad(NULL, 1.0) != HTM_OK,
               "htm_s2cpoly_pad() should have failed");
    HTM_ASSERT(htm_s2cpoly_pad(poly, -1.0) != HTM_OK,
               "htm_s2cpoly_pad() should have failed");
    HTM_ASSERT(htm_s2cpoly_pad(poly, 0.0) == HTM_OK,
               "htm_s2cpoly_pad() failed");
    i = sizeof(struct htm_s2cpoly) + 2 * poly->n * sizeof(struct htm_v3);
    HTM_ASSERT(memcmp(poly, clone, i) == 0, 
               "htm_s2cpoly_pad() with radius 0.0 modified input polygon");
    HTM_ASSERT(htm_s2cpoly_pad(poly, -1.0) != HTM_OK,
               "htm_s2cpoly_pad() should have failed");
    HTM_ASSERT(htm_s2cpoly_pad(poly, 1.0) == HTM_OK,
               "htm_s2cpoly_pad() failed");
    HTM_ASSERT(poly->n == clone->n, "htm_s2cpoly_pad() created/deleted vertices?!");
    free(points);
    /* check that circles of radius almost 1 around each original vertex
       are inside the padded polygon */
    for (i = 0; i < clone->n; ++i) {
        points = gen_circles(&clone->ve[i], 1.0 - 1.0/HTM_ARCSEC_PER_DEG, 1000);
        for (j = 0; j < 1000; ++j) {
            HTM_ASSERT(htm_s2cpoly_cv3(poly, &points[j]) == 1,
                       "htm_s2cpoly_pad() or htm_s2cpoly_cv3() failed");
        }
        free(points);
    }
    free(clone);
    free(poly);
    /* test with large vertex count */
    poly = htm_s2cpoly_ngon(&z, 0.1, 1000, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
    clone = htm_s2cpoly_ngon(&z, 0.1 + 0.01, 1000, &err);
    HTM_ASSERT(clone != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
    HTM_ASSERT(htm_s2cpoly_pad(poly, 0.01) == HTM_OK,
               "htm_s2cpoly_pad() failed");
    for (i = 0; i < 1000; ++i) {
        HTM_ASSERT(v3_almost_equal(&poly->ve[i], &clone->ve[i], 1.0e-08),
                   "htm_s2cpoly_pad() failed");
    }
    free(poly);
    free(clone);
 
    /* test with overly large padding radius */
    poly = htm_s2cpoly_ngon(&z, 45.0, 4, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
    HTM_ASSERT(htm_s2cpoly_pad(poly, 75.0) != HTM_OK,
               "htm_s2cpoly_pad() should have failed");
    free(poly);
    poly = htm_s2cpoly_ngon(&z, 80.0, 128, &err);
    HTM_ASSERT(poly != NULL && err == HTM_OK, "htm_s2cpoly_ngon() failed");
    HTM_ASSERT(htm_s2cpoly_pad(poly, 20.0) != HTM_OK,
               "htm_s2cpoly_pad() should have failed");
    free(poly);
}


/*  Simple hemisphericality test cases.
 */
static void test_hemis()
{
    struct htm_v3 points[6];
    int hemis;
    enum htm_errcode err;

    hemis = htm_v3_hemispherical(NULL, 1, &err);
    HTM_ASSERT(hemis == 0 && err != HTM_OK,
               "htm_v3_hemispherical() should have failed");
    hemis = htm_v3_hemispherical(&x, 0, &err);
    HTM_ASSERT(hemis == 0 && err != HTM_OK,
               "htm_v3_hemispherical() should have failed");
    HTM_ASSERT(htm_v3_hemispherical(&x, 1, &err), "htm_v3_hemispherical() failed");
    HTM_ASSERT(htm_v3_hemispherical(&y, 1, &err), "htm_v3_hemispherical() failed");
    HTM_ASSERT(htm_v3_hemispherical(&z, 1, &err), "htm_v3_hemispherical() failed");
    HTM_ASSERT(htm_v3_hemispherical(&nx, 1, &err), "htm_v3_hemispherical() failed");
    HTM_ASSERT(htm_v3_hemispherical(&ny, 1, &err), "htm_v3_hemispherical() failed");
    HTM_ASSERT(htm_v3_hemispherical(&nz, 1, &err), "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = y; points[2] = z;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = y; points[2] = nz;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = ny; points[2] = z;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = ny; points[2] = nz;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = nx; points[1] = y; points[2] = z;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = nx; points[1] = y; points[2] = nz;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = nx; points[1] = ny; points[2] = z;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = nx; points[1] = ny; points[2] = nz;
    HTM_ASSERT(htm_v3_hemispherical(points, 3, &err),
               "htm_v3_hemispherical() failed");
    points[0] = x;
    HTM_ASSERT(htm_v3_hemispherical(points, 1, &err),
               "htm_v3_hemispherical() failed");
    points[0] = y;
    HTM_ASSERT(htm_v3_hemispherical(points, 1, &err),
               "htm_v3_hemispherical() failed");
    points[0] = z;
    HTM_ASSERT(htm_v3_hemispherical(points, 1, &err),
               "htm_v3_hemispherical() failed");
    points[0] = nx;
    HTM_ASSERT(htm_v3_hemispherical(points, 1, &err),
               "htm_v3_hemispherical() failed");
    points[0] = ny;
    HTM_ASSERT(htm_v3_hemispherical(points, 1, &err),
               "htm_v3_hemispherical() failed");
    points[0] = nz;
    HTM_ASSERT(htm_v3_hemispherical(points, 1, &err),
               "htm_v3_hemispherical() failed");

    points[0] = x; points[1] = nx;
    hemis = htm_v3_hemispherical(points, 2, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = y; points[1] = ny;
    hemis = htm_v3_hemispherical(points, 2, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = z; points[1] = nz;
    hemis = htm_v3_hemispherical(points, 2, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");

    points[0] = x; points[1] = nx; points[2] = z;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = nx; points[2] = y;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = nx; points[2] = nz;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = x; points[1] = nx; points[2] = ny;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");

    points[0] = y; points[1] = ny; points[2] = x;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = y; points[1] = ny; points[2] = z;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = y; points[1] = ny; points[2] = nx;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
    points[0] = y; points[1] = ny; points[2] = nz;
    hemis = htm_v3_hemispherical(points, 3, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");

    points[0] =  x; points[1] =  y; points[2] =  z;
    points[3] = nx; points[4] = ny; points[5] = nz;
    hemis = htm_v3_hemispherical(points, 6, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");

    points[0].x = 0.97097124047361949;
    points[0].y = -0.15092001616163719;
    points[0].z = -0.18557477979211451;
    points[1].x = 0.9872559769915108;
    points[1].y = 0.032844295708767957;
    points[1].z = 0.15571412310363025;
    points[2].x = 0.9721056759425426;
    points[2].y = -0.1930472758520447;
    points[2].z = 0.13320399425841953;
    points[3].x = -0.99379988981039358;
    points[3].y = 0.10551497413174188;
    points[3].z = -0.035048099047274975;
    hemis = htm_v3_hemispherical(points, 4, &err);
    HTM_ASSERT(err == HTM_OK && !hemis, "htm_v3_hemispherical() failed");
}


static void test_hemis_rand()
{
    size_t i, j;
    /* test with randomly generated point clouds */
    for (i = 0; i < NCENTERS; ++i) {
        struct htm_v3 v;
        htm_v3_normalize(&v, &centers[i]);
        for (j = 0; j < NLENGTHS; ++j) {
            size_t n = lengths[j];
            struct htm_v3 *points = gen_hemis(&v, n);
            int hemis;
            enum htm_errcode err;

            hemis = htm_v3_hemispherical(points, n, &err);
            HTM_ASSERT(err == HTM_OK && hemis,
                       "htm_v3_hemispherical() failed");
            gen_opposing(points, n);
            hemis = htm_v3_hemispherical(points, n + 1, &err);
            HTM_ASSERT(err == HTM_OK && !hemis,
                       "htm_v3_hemispherical() failed");
            free(points);
        }
    }
    /* test with points on circles */
    for (i = 0; i < NCENTERS; ++i) {
        struct htm_v3 v;
        htm_v3_normalize(&v, &centers[i]);
        for (j = 0; j < NLENGTHS; ++j) {
            size_t n = lengths[j];
            struct htm_v3 *points = gen_circles(&v, 90.0 - 0.002/HTM_ARCSEC_PER_DEG, n);
            int hemis;
            enum htm_errcode err;
            hemis = htm_v3_hemispherical(points, n, &err);
            HTM_ASSERT(err == HTM_OK && hemis,
                       "htm_v3_hemispherical() failed");
            hemis = htm_v3_hemispherical(points + n, n, &err);
            HTM_ASSERT(err == HTM_OK && hemis,
                       "htm_v3_hemispherical() failed");
            hemis = htm_v3_hemispherical(points, 2 * n, &err);
            HTM_ASSERT(err == HTM_OK && !hemis,
                       "htm_v3_hemispherical() failed");
            free(points);
        }
    }
}


/*  Generate n points on a periodic hypotrochoid.
 */
static struct htm_v3 * gen_periodic_hypotrochoid(const struct htm_v3 *c,
                                                 size_t R,
                                                 size_t r,
                                                 size_t d,
                                                 size_t n)
{
    struct htm_v3 cen, north, east;
    struct htm_v3 *points;
    size_t period, i;

    period = (2 * r) / (R - r);
    HTM_ASSERT(R > r && period * (R - r) == 2 * r,
               "parameters do not yield a periodic hypotrochoid");
    htm_v3_normalize(&cen, c);
    htm_v3_ne(&north, &east, &cen);
    if (period % 2 != 0) {
        period *= 2;
    }
    points = (struct htm_v3 *) malloc(n * sizeof(struct htm_v3));
    HTM_ASSERT(points != NULL, "memory allocation failed");
    for (i = 0; i < n; ++i) {
        struct htm_v3 tmp;
        double x, y, a = (i * period) * M_PI / n;
        double sina = sin(a);
        double cosa = cos(a);
        x = ((R - r) * cosa + d * cos((R - r) * a / r));
        y = ((R - r) * sina - d * sin((R - r) * a / r));
        htm_v3_mul(&tmp, &north, x);
        htm_v3_add(&points[i], &cen, &tmp);
        htm_v3_mul(&tmp, &east, y);
        htm_v3_add(&points[i], &points[i], &tmp);
        htm_v3_normalize(&points[i], &points[i]);
    }
    return points;
}
                                                 

static void reverse(struct htm_v3 *points, size_t n)
{
    size_t i, j;
    for (i = 0, j = n - 1; i < j; ++i, --j) {
        struct htm_v3 tmp = points[i];
        points[i] = points[j];
        points[j] = tmp;
    }
}


static void doubleup(struct htm_v3 *points, size_t n)
{
    size_t i;
    for (i = n; i > 0; --i) {
        points[2*i - 2] = points[i - 1];
        points[2*i - 1] = points[i - 1];
    }
}
    

static void test_convex()
{
    struct htm_v3 verts[4];
    struct htm_v3 *points;
    size_t i, j;
    enum htm_errcode err;
    int cvx;

    for (i = 0; i < NCENTERS; ++i) {
        struct htm_v3 v;
        htm_v3_normalize(&v, &centers[i]);
        for (j = 0; j < NLENGTHS; ++j) {
            const size_t n = lengths[j];
            points = gen_circles(&v, htm_rand()*89.0 + 0.01, n);
            HTM_ASSERT(htm_v3_convex(points, n, &err) == 1,
                       "htm_v3_convex() failed %d,%d", (int)i, (int)j);
            cvx = htm_v3_convex(points, 2 * n, &err);
            HTM_ASSERT(err == HTM_OK && cvx == 0, "htm_v3_convex() failed");
            reverse(points, n);
            HTM_ASSERT(htm_v3_convex(points, n, &err) == -1,
                       "htm_v3_convex() failed");
            /* test with duplicate vertices */
            doubleup(points, n);
            cvx = htm_v3_convex(points, 2 * n, &err);
            HTM_ASSERT(err == HTM_EDEGEN && cvx == 0, "htm_v3_convex() failed");
            free(points);
        }
    }
    verts[0] = x;
    verts[1] = y;
    verts[2] = z;
    verts[3].x = 1.0;
    verts[3].y = 0.5;
    verts[3].z = 2.0;
    htm_v3_normalize(&verts[3], &verts[3]);
    cvx = htm_v3_convex(NULL, 3, &err);
    HTM_ASSERT(err != HTM_OK && cvx == 0, "htm_v3_convex() should have failed");
    cvx = htm_v3_convex(verts, 2, &err);
    HTM_ASSERT(err != HTM_OK && cvx == 0, "htm_v3_convex() should have failed");
    cvx = htm_v3_convex(verts, 4, &err);
    HTM_ASSERT(err == HTM_OK && cvx == 0, "htm_v3_convex() failed");
    /* consecutive edges of the following hypotrochoids look convex,
       but the polygon formed by them is self-intersecting - it winds
       around the centroid multiple times. */
    points = gen_periodic_hypotrochoid(&xyz, 5, 3, 5, 100);
    cvx = htm_v3_convex(points, 100, &err);
    HTM_ASSERT(err == HTM_OK && cvx == 0, "htm_v3_convex() failed");
    free(points);
    points = gen_periodic_hypotrochoid(&xyz, 6, 4, 6, 3000);
    cvx = htm_v3_convex(points, 3000, &err);
    HTM_ASSERT(err == HTM_OK && cvx == 0, "htm_v3_convex() failed");
    free(points);
    /* test with degenerate points */
    verts[0] = x;
    verts[1] = x;
    verts[2] = x;
    cvx = htm_v3_convex(verts, 3, &err);
    HTM_ASSERT(err != HTM_OK && cvx == 0, "htm_v3_convex() should have failed");
    verts[2] = y;
    cvx = htm_v3_convex(verts, 3, &err);
    HTM_ASSERT(err != HTM_OK && cvx == 0, "htm_v3_convex() should have failed");
}


static int check_hull(const struct htm_s2cpoly *hull,
                      const struct htm_v3 *points,
                      size_t n)
{
    size_t i, j;
    size_t nh = hull->n;

    for (i = 0; i < n; ++i) {
        for (j = 0; j < nh; ++j) {
            if (v3_equal(&points[i], &hull->ve[j])) {
                break;
            }
        }
        if (j < hull->n) {
            continue;
        }
        /* point does not belong to hull */
        for (j = nh; j < 2 * nh; ++j) {
            if (htm_v3_dot(&points[i], &hull->ve[nh]) < -2.0e-15) {
                return 0; 
            }
        }
    }
    return 1;
}


static void shuffle(struct htm_v3 *points, size_t n)
{
    struct htm_v3 tmp;
    size_t i;

    for (i = 0; i < 7*n; ++i) {
        size_t j = (size_t) (htm_rand() * n);
        size_t k = (size_t) (htm_rand() * n);
        if (j >= n) {
            j = n - 1;
        }
        if (k >= n) {
            k = n - 1;
        }
        tmp = points[j];
        points[j] = points[k];
        points[k] = tmp; 
    }
}


static void test_hull()
{
    struct htm_v3 v;
    struct htm_v3 *points;
    struct htm_s2cpoly *hull;
    enum htm_errcode err;
    size_t i, j, k;

    HTM_ASSERT(htm_s2cpoly_hull(NULL, 3, &err) == NULL,
               "htm_s2cpoly_hull() should have failed");
    points = gen_hemis(&xyz, 32);
    HTM_ASSERT(htm_s2cpoly_hull(points, 2, &err) == NULL,
               "htm_s2cpoly_hull() should have failed");
    /* test with small numbers of points */
    hull = htm_s2cpoly_hull(points, 32, &err);
    HTM_ASSERT(hull != NULL && err == HTM_OK, "htm_s2cpoly_hull() failed");
    HTM_ASSERT(htm_v3_convex(hull->ve, hull->n, &err) == 1,
               "htm_s2cpoly_hull() failed");
    HTM_ASSERT(check_hull(hull, points, 32) == 1, "htm_s2cpoly_hull() failed");
    free(hull);
    points[0] = x; points[1] = y; points[2] = z;
    hull = htm_s2cpoly_hull(points, 3, &err);
    HTM_ASSERT(hull != NULL && err == HTM_OK, "htm_s2cpoly_hull() failed");
    HTM_ASSERT(htm_v3_convex(hull->ve, hull->n, &err) == 1,
               "htm_s2cpoly_hull() failed");
    HTM_ASSERT(check_hull(hull, points, 3) == 1, "htm_s2cpoly_hull() failed");
    free(hull);
    /* check with degenerate point sets */
    points[0] = x;
    points[1] = x;
    points[2] = x;
    hull = htm_s2cpoly_hull(points, 3, &err);
    HTM_ASSERT(hull == NULL && err != HTM_OK,
               "htm_s2cpoly_hull() should have failed");
    points[2] = y;
    hull = htm_s2cpoly_hull(points, 3, &err);
    HTM_ASSERT(hull == NULL && err != HTM_OK,
               "htm_s2cpoly_hull() should have failed");
    htm_v3_add(&points[1], &x, &y);
    htm_v3_normalize(&points[1], &points[1]);
    hull = htm_s2cpoly_hull(points, 3, &err);
    HTM_ASSERT(hull == NULL && err != HTM_OK,
               "htm_s2cpoly_hull() should have failed");
    free(points); 

    v.x = 1.0; v.y = 1.0; v.z = -1.0;
    points = gen_periodic_hypotrochoid(&v, 15, 13, 15, 1000);
    hull = htm_s2cpoly_hull(points, 1000, &err);
    HTM_ASSERT(hull != NULL && err == HTM_OK, "htm_s2cpoly_hull() failed");
    HTM_ASSERT(htm_v3_convex(hull->ve, hull->n, &err) == 1,
               "htm_s2cpoly_hull() failed");
    HTM_ASSERT(check_hull(hull, points, 1000) == 1, "htm_s2cpoly_hull() failed");
    free(hull);
    shuffle(points, 1000);
    hull = htm_s2cpoly_hull(points, 1000, &err);
    HTM_ASSERT(hull != NULL && err == HTM_OK, "htm_s2cpoly_hull() failed");
    HTM_ASSERT(htm_v3_convex(hull->ve, hull->n, &err) == 1,
               "htm_s2cpoly_hull() failed");
    HTM_ASSERT(check_hull(hull, points, 1000) == 1, "htm_s2cpoly_hull() failed");
    free(hull);
    free(points);

    /* test with duplicate vertices */
    points = gen_circles(&x, 10.0, 100);
    memcpy(points + 100, points, 100 * sizeof(struct htm_v3));
    for (k = 0; k < 2; ++k) {
        hull = htm_s2cpoly_hull(points, 200, &err);
        HTM_ASSERT(hull != NULL && err == HTM_OK, "htm_s2cpoly_hull() failed");
        HTM_ASSERT(htm_v3_convex(hull->ve, hull->n, &err) == 1,
                   "htm_s2cpoly_hull() failed");
        HTM_ASSERT(hull->n == 100, "htm_s2cpoly_hull() failed");
        for (i = 0; i < 100; ++i) {
            for (j = 0; j < 100; ++j) {
                if (v3_equal(&points[i], &hull->ve[j])) {
                    break;
                }
            }
            HTM_ASSERT(j < 100, "htm_s2cpoly_hull() failed");
        }
        free(hull);
        shuffle(points, 200);
    }
    free(points);

    /* test with non-hemispherical points */
    v.x = 1.0; v.y = -1.0; v.z = 0.5;
    points = gen_hemis(&v, 1000);
    gen_opposing(points, 1000);
    hull = htm_s2cpoly_hull(points, 1001, &err);
    HTM_ASSERT(hull == NULL && err == HTM_EHEMIS,
               "htm_s2cpoly_hull() should have failed");
    free(points);
}


int main(int argc HTM_UNUSED, char **argv HTM_UNUSED)
{
    htm_seed(987654321UL);
    test_utils();
    test_v3_basics();
    test_sc();
    test_v3_ne();
    test_v3_rot();
    test_ang_dist();
    test_s2cpoly();
    test_pad();
    test_hemis();
    test_hemis_rand();
    test_convex();
    test_hull();
    return 0;
}

