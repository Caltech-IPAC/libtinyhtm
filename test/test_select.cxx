/** \file
    \brief  Unit tests for selection algorithms 

    \authors Serge Monkewitz
    \copyright IPAC/Caltech
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinyhtm/select.h"
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


static size_t uniform(size_t n) {
    size_t u = (size_t) (n * htm_rand());
    return u >= n ? n - 1 : u;
}

static void shuffle(double *array, size_t n) {
    size_t i, j, k;
    double tmp;
    for (i = 0; i < n; ++i) {
        j = uniform(n);
        k = uniform(n);
        tmp = array[j];
        array[j] = array[k];
        array[k] = tmp;
    }
}

struct {
    size_t n;
    size_t nperms;
    double array[10];
} permgen;


static size_t factorial(size_t n) {
    size_t f = n;
    for (--n; n > 1; --n) {
        f *= n;
    }
    return f;
}


static void permgen_init(size_t n) {
    size_t i;

    HTM_ASSERT(n > 0 && n <= 10, "Invalid permutation generator paramaters");
    permgen.n = n;
    permgen.nperms = factorial(n);
    for (i = 0; i < n; ++i) {
        permgen.array[i] = i;
    }
}


static void permgen_next() {
    double tmp;
    size_t j, k;

    for (j = permgen.n - 2; permgen.array[j] > permgen.array[j + 1]; --j) {
        if (j == 0) {
            return; /* last permutation already computed */
        }
    }
    for (k = permgen.n - 1; permgen.array[j] > permgen.array[k]; --k) { }
    tmp = permgen.array[k];
    permgen.array[k] = permgen.array[j];
    permgen.array[j] = tmp;
    for (j = j + 1, k = permgen.n - 1; j < k; ++j, --k) {
        tmp = permgen.array[k];
        permgen.array[k] = permgen.array[j];
        permgen.array[j] = tmp;
    }
}


static void test_select(double (*select)(double *, size_t, size_t)) {
    static const size_t MAX_N = 1024*1024;

    double *array;
    double expected, actual, emin;
    size_t n;

    array = (double *) malloc(MAX_N * sizeof(double));
    HTM_ASSERT(array != NULL, "memory allocation failed");

    /* test all possible permutations of n distinct values, for n in [1, 10] */
    for (n = 1; n <= 10; ++n) {
        size_t i;
        permgen_init(n);
        expected = n >> 1;
        for (i = 0; i < permgen.nperms; ++i, permgen_next()) {
            memcpy(array, permgen.array, n * sizeof(double));
            actual = (*select)(array, n, n >> 1);
            HTM_ASSERT(expected == actual, "median failed on permutation "
                "%llu of %llu permutations of 0 .. %d", (unsigned long long) i,
                (unsigned long long) permgen.nperms, (int) (n - 1));
            memcpy(array, permgen.array, n * sizeof(double));
            emin = htm_min(array, n); 
            actual = (*select)(array, n, 0);
            HTM_ASSERT(actual == emin, "minimal element selection failed");
        }
    }

    /* test sequences of identical values */
    for (n = 1; n < 100; ++n) {
        size_t i;
        for (i = 0; i < n; ++i) {
            array[i] = 1.0;
        }
        expected = 1.0;
        actual = (*select)(array, n, n >> 2);
        HTM_ASSERT(expected == actual, "quartile failed on array of "
            "%llu identical values", (unsigned long long) n);
    }

    /* test sequence of ascending values */
    for (n = 1; n <= MAX_N; n *= 2) {
        size_t i;
        for (i = 0; i < n; ++i) {
            array[i] = i;
        }
        expected = n >> 1;
        actual = (*select)(array, n, n >> 1);
        HTM_ASSERT(expected == actual, "median failed on array of "
            "%llu ascending values", (unsigned long long) n);
    }

    /* test sequence of descending values */
    for (n = 1; n <= MAX_N; n *= 2) {
        size_t i;
        for (i = 0; i < n; ++i) {
            array[i] = n - 1 - i;
        }
        expected = n >> 1;
        actual = (*select)(array, n, n >> 1);
        HTM_ASSERT(expected == actual, "median failed on array of "
            "%llu descending values", (unsigned long long) n);
    }

    /* Test randomly shuffled sequences of values */
    for (n = 1; n <= MAX_N; n *= 2) {
        size_t i;
        for (i = 0; i < n; ++i) {
            array[i] = n - 1 - i;
        }
        shuffle(array, n);
        expected = n >> 1;
        actual = (*select)(array, n, n >> 1);
        HTM_ASSERT(expected == actual, "median failed on array of "
            "%llu shuffled distinct values", (unsigned long long) n);
    }

    /* Test sequences containing duplicate values */
    for (n = 1; n <= MAX_N; n = 5*n/4 + 1) {
        size_t i, j;
        for (i = 0, j = 0; i < n; ++i) {
            array[i] = j;
            if (htm_rand() > 0.7) {
                ++j;
            }
        }
        expected = array[n >> 1];
        shuffle(array, n);
        actual = (*select)(array, n, n >> 1);
        HTM_ASSERT(expected == actual, "median failed on array of "
            "%llu shuffled values with duplicates", (unsigned long long) n);
     }
     free(array);
}


int main(int argc HTM_UNUSED, char **argv HTM_UNUSED) {
    htm_seed(123454321UL);
    test_select(&htm_select);
    test_select(&htm_selectmm);
    return 0;
}

