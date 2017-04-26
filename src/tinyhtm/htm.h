
//  $Id: htm.h,v 20130613.99132738 2013/06/13 20:27:38 modell ipac $

/** \file
    \brief      Minimalistic functions and types for HTM indexing.

    This software is based on work by A. Szalay, T. Budavari,
    G. Fekete at The Johns Hopkins University, and Jim Gray,
    Microsoft Research. See the following links for more information:

    http://voservices.net/spherical/
    http://adsabs.harvard.edu/abs/2010PASP..122.1375B

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#ifndef HTM_HTM_H
#define HTM_HTM_H

#include <string.h>
#include <inttypes.h>

#include "geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IDSIZE 64

//  cf. alternate in alt_htm_level()
#define IDHIGHBIT 0x8000000000000000LL
#define IDHIGHBIT2 0x4000000000000000LL

#define HTM_INVALID_ID -1

#ifdef USE_128
#define NB 128
#define ULG U128_TYPE
#define HTM_DEC_MAX_LEVEL 36
#define HTM_MAX_LEVEL 48
#define LIM_BIG_INT LIM_UINT128
#define SHIFT_BY 112
#else
#define NB 64
#define ULG U64_TYPE
#define HTM_DEC_MAX_LEVEL 18
#define HTM_MAX_LEVEL 24
#define LIM_BIG_INT LIM_UINT64
#define SHIFT_BY 56
#endif

#define SYS_SIZ_LIM 64

#define ULL ULG

#define LLI long long int
#define ULLI unsigned long long int

#define UINT64_MAX_VAL 18446744073709551615ULL

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY (x)

#define HTM_OLD_LEV 7
#define HTM_NEW_LEV 20

#define EOSC 0

#define NS (NB / 2)
#define BC (NB / 4)
#define NC (NB + BC)
#define SB0 8
#define SB1 32
#define SB2 64

//#define A_UINT128_C(value)    value##uLLL
#define A_UINT64_C(value) value##uLL
#define A_UINT_LG_C(value) (ULL)##value

//    #define U128_TYPE             uint128_t
//    #define U64_TYPE              uint64_t

#define U128_TYPE I128_TYPE
#define U64_TYPE I64_TYPE

#define I128_TYPE int128_t
#define I64_TYPE int64_t

#define LIM_UINT64 1000000000000000000ULL
//  #define LIM_UINT128           1000000000000000000000000000000000000ULLL
//  #define LIM_UINT128           ( 0x1000000000000000ULL << SYS_SIZ_LIM )
#define LIM_UINT128 ((uint128_t)(1.0e39))

#define P10_UINT64 10000000000000000000ULL
#define E10_UINT64 19 //  # zeros

#ifndef FALSE
#define FALSE (0)
#define TRUE (!FALSE)
#endif

/** \defgroup htm HTM Indexing
    @{
  */

/** Maximum HTM tree subdivision level */
#define HTM_MAX_LEVEL 24

/** Maximum level of a decimal index stored in a 64 bit signed integer. */
#define HTM_DEC_MAX_LEVEL 18

/**  Root triangle numbers. The HTM ID of a root triangle is its number plus 8.
  */
enum htm_root
{
  HTM_S0 = 0,
  HTM_S1 = 1,
  HTM_S2 = 2,
  HTM_S3 = 3,
  HTM_N0 = 4,
  HTM_N1 = 5,
  HTM_N2 = 6,
  HTM_N3 = 7,
  HTM_NROOTS = 8
};

/** A sorted list of 64 bit integer ranges.
  */
struct htm_ids
{
  size_t n;                 /**< Number of ranges in list */
  size_t cap;               /**< Capacity of the range list */
  struct htm_range range[]; /**< Ranges <tt>[min_i, max_i]</tt>, with
                                 <tt>min_i <= max_i</tt> and
                                 <tt>min_j > max_i</tt> for all
                                 <tt>j > i</tt>. */
};

/** A position and a payload.
  */
struct htm_v3p
{
  struct htm_v3 v; /**< Position. */
  void *payload;   /**< Pointer to payload. */
} HTM_ALIGNED (16);

/** A structure describing the geometry of an HTM triangle (aka a trixel).
  */
struct htm_tri
{
  struct htm_v3 verts[3]; /**< Triangle vertices. */
  struct htm_v3 center;   /**< Triangle center. */
  double radius;          /**< Bounding circle radius (degrees). */
  int64_t id;             /**< HTM id. */
  int level;              /**< HTM level. */
};

/*  Computes the normalized average of two input vertices.
 */
HTM_INLINE void _htm_vertex (struct htm_v3 *out, const struct htm_v3 *v1,
                             const struct htm_v3 *v2)
{
  htm_v3_add (out, v1, v2);
  htm_v3_normalize (out, out);
}

/** Computes an HTM ID for a position.

    \return The HTM ID for \p point, or 0 if \p point is \c NULL or
             \p level is not in the range <tt>[0, HTM_MAX_LEVEL]</tt>.
  */
int64_t htm_v3_id (const struct htm_v3 *point, int level);

/** Computes HTM IDs for a list of positions with payloads. Positions
    and payloads are sorted by HTM ID during the id generation process.

    \return
            - HTM_ENULLPTR  if \p points or \p ids is NULL.
            - HTM_ELEN      if \p n is 0.
            - HTM_ELEVEL    if level is not in <tt>[0, HTM_MAX_LEVEL]</tt>.
            - HTM_OK        on success.
  */
enum htm_errcode htm_v3p_idsort (struct htm_v3p *points, int64_t *ids,
                                 size_t n, int level);

/** Returns the HTM subdivision level of the given ID,
    or -1 if the ID is invalid.
  */
int htm_level (int64_t id);

/** Returns the HTM subdivision level of the given ID,
    or -1 if the ID is invalid.
  */
int alt_htm_level (int64_t htmid);

/** Returns the HTM subdivision level of the given ID,
    trying first Binary then Decimal encoding,
    or -1 if the ID is invalid.
  */
int full_alt_htm_level (int64_t htmid);

/** Computes and stores the attributes of the HTM triangle with the given id.

    \return
            - HTM_ENULLPTR  if \p tri is NULL.
            - HTM_EID       if \p id is invalid.
            - HTM_OK        on success.
  */
enum htm_errcode htm_tri_init (struct htm_tri *tri, int64_t id);

/** Computes and stores the attributes of the HTM triangle with the given id.
    Uses alternate level detection, including encoding correction.

    \return
            - HTM_ENULLPTR  if \p tri is NULL.
            - HTM_EID       if \p id is invalid.
            - HTM_OK        on success.
  */
enum htm_errcode alt_htm_tri_init (struct htm_tri *tri, int64_t id);

/** Returns a list of HTM ID ranges corresponding to the HTM triangles
    overlapping the given circle.

    A range list can be de-allocated by passing its pointer
    to free().

    \param[in] ids          Existing range list or NULL. If \p ids is NULL,
                            a fresh range list is allocated and returned.
                            If it is non-NULL, its entries are removed but
                            its memory is reused. This can be used to avoid
                            \c malloc/realloc costs when this function is
                            being called inside of a loop.
    \param[in] center       Spherical circle center.
    \param[in] radius       Spherical circle radius (degrees).
    \param[in] level        Subdivision level, in range
                            <tt>[0, HTM_MAX_LEVEL]</tt>.
    \param[in] maxranges    Maximum number of ranges to return. When too many
                            ranges are generated, the effective subdivision
                            level of HTM ids is reduced. Since two consecutive
                            ranges that cannot be merged at level L may become
                            mergable at level L-n, this "coarsening" cuts down
                            on the number of ranges, but makes the range list
                            a poorer approximation to the input geometry. Note
                            that for arbitrary input geometry, up to 4 ranges
                            may be generated no matter what the subdivision
                            level is. So for maxranges < 4, the requested
                            bound may not be achieved.
    \param[out] err         If an error occurs, \p *err is set to indicate
                            the reason for the failure.

    \return     A list of HTM ID ranges for the HTM triangles overlapping the
                given circle. A NULL pointer is returned in case of error, and
                \p *err is set to indicate the reason for the failure.

                Note that the input range list may be reallocated (to grow its
                capacity), and so the input range list pointer may no longer
                point to valid memory. Always replace the input pointer with
                the return value of this function!

                If a failure occurs, this function will free the memory
                associated with the range list (even it came from a
                non-NULL input pointer).
  */
struct htm_ids *htm_s2circle_ids (struct htm_ids *ids,
                                  const struct htm_v3 *center, double radius,
                                  int level, size_t maxranges,
                                  enum htm_errcode *err);

/** Returns a list of HTM ID ranges corresponding to the HTM triangles
    overlapping the given ellipse.

    A range list can be de-allocated by passing its pointer
    to free().

    \param[in] ids          Existing range list or NULL. If \p ids is NULL,
                            a fresh range list is allocated and returned.
                            If it is non-NULL, its entries are removed but
                            its memory is reused. This can be used to avoid
                            \c malloc/realloc costs when this function is
                            being called inside of a loop.
    \param[in] ellipse      Spherical ellipse.
    \param[in] level        Subdivision level, in range
                            <tt>[0, HTM_MAX_LEVEL]</tt>.
    \param[in] maxranges    Maximum number of ranges to return. When too many
                            ranges are generated, the effective subdivision
                            level of HTM ids is reduced. Since two consecutive
                            ranges that cannot be merged at level L may become
                            mergable at level L-n, this "coarsening" cuts down
                            on the number of ranges, but makes the range list
                            a poorer approximation to the input geometry. Note
                            that for arbitrary input geometry, up to 4 ranges
                            may be generated no matter what the subdivision
                            level is. So for maxranges < 4, the requested
                            bound may not be achieved.
    \param[out] err         If an error occurs, \p *err is set to indicate
                            the reason for the failure.

    \return     A list of HTM ID ranges for the HTM triangles overlapping the
                given ellipse. A NULL pointer is returned in case of error,
                and \p *err is set to indicate the reason for the failure.

                Note that the input range list may be reallocated (to grow its
                capacity), and so the input range list pointer may no longer
                point to valid memory. Always replace the input pointer with
                the return value of this function!

                If a failure occurs, this function will free the memory
                associated with the range list (even it came from a
                non-NULL input pointer).
  */
struct htm_ids *htm_s2ellipse_ids (struct htm_ids *ids,
                                   const struct htm_s2ellipse *ellipse,
                                   int level, size_t maxranges,
                                   enum htm_errcode *err);

/*  HTM encoding  */

//  probably decimal
int prob_decimal (ULL in);

//  definitely biinary
int def_binary (ULL in);

/** Returns a list of HTM ID ranges corresponding to the HTM triangles
    overlapping the given spherical convex polygon.

    A range list can be de-allocated by passing its pointer
    to free().

    \param[in] ids          Existing range list or NULL. If \p ids is NULL,
                            a fresh range list is allocated and returned.
                            If it is non-NULL, its entries are removed but
                            its memory is reused. This can be used to avoid
                            \c malloc/realloc costs when this function is
                            being called inside of a loop.
    \param[in] poly         Spherical convex polygon.
    \param[in] level        Subdivision level, in range
                            <tt>[0, HTM_MAX_LEVEL]</tt>.
    \param[in] maxranges    Maximum number of ranges to return. When too many
                            ranges are generated, the effective subdivision
                            level of HTM ids is reduced. Since two consecutive
                            ranges that cannot be merged at level L may become
                            mergable at level L-n, this "coarsening" cuts down
                            on the number of ranges, but makes the range list
                            a poorer approximation to the input geometry. Note
                            that for arbitrary input geometry, up to 4 ranges
                            may be generated no matter what the subdivision
                            level is. So for maxranges < 4, the requested
                            bound may not be achieved.
    \param[out] err         If an error occurs, \p *err is set to indicate
                            the reason for the failure.

    \return     A list of HTM ID ranges for the HTM triangles overlapping the
                given polygon. A NULL pointer is returned in case of error,
                and \p *err is set to indicate the reason for the failure.

                Note that the input range list may be reallocated (to grow its
                capacity), and so the input range list pointer may no longer
                point to valid memory. Always replace the input pointer with
                the return value of this function!

                If a failure occurs, this function will free the memory
                associated with the range list (even it came from a
                non-NULL input pointer).
  */
struct htm_ids *htm_s2cpoly_ids (struct htm_ids *ids,
                                 const struct htm_s2cpoly *poly, int level,
                                 size_t maxranges, enum htm_errcode *err);

/** Converts an HTM ID as returned by the various indexing functions to
    decimal form. Returns 0 if the input ID is invalid.

    This function maps IDs produced by this library to IDs that are compatible
    with those currently stored in IRSA database tables.
  */
int64_t htm_idtodec (int64_t id);

int64_t alt_htm_idtodec (int64_t id);

/** Converts an HTM ID, taken to be in decimal representation,
    to binary form. Returns 0 if the input ID is invalid.

    This function maps IDs currently stored in IRSA database tables
    (Decimal encoded) to a Binary representation.
  */
int64_t htm_idfrdec (int64_t id);

int64_t alt_htm_idfrdec (int64_t id);

int dec_dig (int64_t num, int pow);

int n_dec_digs (int64_t num);

char *bin_txt (int64_t rx);

char *str_big_num (int64_t big_num);

void sho_bin (int64_t number);

/** @}
  */

#ifdef __cplusplus
}
#endif

#endif /* HTM_HTM_H */

//  vi: set tabstop=4 shiftwidth=4 expandtab :
