/** \file
    \brief      Minimalistic HTM indexing implementation.

    This software is based on work by A. Szalay, T. Budavari,
    G. Fekete at The Johns Hopkins University, and Jim Gray,
    Microsoft Research. See the following links for more information:

    http://voservices.net/spherical/
    http://adsabs.harvard.edu/abs/2010PASP..122.1375B

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "tinyhtm/tree.h"
#include "tinyhtm/varint.h"
#include "htm.hxx"

#include <sys/mman.h>

extern "C" {

//  HTM level calculation
int htm_level (int64_t id)
{

  uint64_t x = (uint64_t)id;
  int l;

  if (id < 8)
    return -1;

  x |= (x >> 1);
  x |= (x >> 2);
  x |= (x >> 4);
  x |= (x >> 8);
  x |= (x >> 16);
  x |= (x >> 32);

  l = htm_popcount (x) - 4;

  //  check that l is even, in range, and that the
  //  4 MSBs of id give a valid root ID (8-15)

  if ((l & 1) != 0 || ((id >> l) & 0x8) == 0 || l > HTM_MAX_LEVEL * 2)
    {
      return -1;
    }

  return l / 2;
}

//  an alternate HTM level calculation
int alt_htm_level (U64_TYPE htmid)
{

  //  mostly ripped from google code SkyWatcherProtocol
  //  http://indi-skywatcherprotocol.googlecode.com/svn-history/r1/trunk/indi-eqmod/align/htm.c
  //  size is the length of the string representing the name of the trixel
  //  the level is then (size-2)

  int ii;
  unsigned long int size = 0;

  // determine index of first set bit

  for (ii = 0; ii < IDSIZE; ii += 2)
    {

      if ((htmid << ii) & IDHIGHBIT)
        break;

      if ((htmid << ii) & IDHIGHBIT2)
        return HTM_INVALID_ID;
    }

  if (0 == htmid)
    return HTM_INVALID_ID;

  size = (IDSIZE - ii) >> 1;

  return (size - 2);
}

//  a further-diverged HTM level calculation
int full_alt_htm_level (U64_TYPE htmid)
{

  int lvl;

  lvl = alt_htm_level (htmid);
  //  printf( "\n * BINARY  Level  %d  ( %lld )\n", lvl, htmid );

  if (0 >= lvl)
    {
      lvl = alt_htm_level (htm_idfrdec (htmid));
      //  printf( "\n * DECIMAL Level  %d  ( %lld )\n", lvl, htmid );
    }

  if (0 >= lvl)
    {
      printf (
          "\n * HTM Level calculation failed trying both Binary & Decimal\n");
      lvl = -1;
    }

  return (lvl);
}

enum htm_errcode htm_tri_init (struct htm_tri *tri, int64_t id)
{
  struct htm_v3 v0, v1, v2;
  struct htm_v3 sv0, sv1, sv2;
  int shift, level;
  enum htm_root r;

  if (tri == NULL)
    {
      return HTM_ENULLPTR;
    }
  level = htm_level (id);
  if (level < 0)
    {
      return HTM_EID;
    }
  tri->id = id;
  tri->level = level;
  shift = 2 * level;
  r = static_cast<htm_root>((id >> shift) & 0x7);
  v0 = *_htm_root_vert[r * 3];
  v1 = *_htm_root_vert[r * 3 + 1];
  v2 = *_htm_root_vert[r * 3 + 2];
  for (shift -= 2; shift >= 0; shift -= 2)
    {
      int child = (id >> shift) & 0x3;
      _htm_vertex (&sv1, &v2, &v0);
      _htm_vertex (&sv2, &v0, &v1);
      _htm_vertex (&sv0, &v1, &v2);
      switch (child)
        {
        case 0:
          v1 = sv2;
          v2 = sv1;
          break;
        case 1:
          v0 = v1;
          v1 = sv0;
          v2 = sv2;
          break;
        case 2:

          v0 = v2;
          v1 = sv1;
          v2 = sv0;
          break;
        case 3:
          v0 = sv0;
          v1 = sv1;
          v2 = sv2;
          break;
        }
    }
  tri->verts[0] = v0;
  tri->verts[1] = v1;
  tri->verts[2] = v2;
  htm_v3_add (&sv0, &v0, &v1);
  htm_v3_add (&sv0, &sv0, &v2);
  htm_v3_normalize (&tri->center, &sv0);
  tri->radius = htm_v3_angsep (&sv0, &v0);
  return HTM_OK;
}

enum htm_errcode alt_htm_tri_init (struct htm_tri *tri, int64_t id)
{

  struct htm_v3 v0, v1, v2;
  struct htm_v3 sv0, sv1, sv2;
  int shift, level;
  enum htm_root r;

  if (tri == NULL)
    {
      return HTM_ENULLPTR;
    }
  //  level = alt_htm_level(id);
  level = full_alt_htm_level (id);
  if (level < 0)
    {
      return HTM_EID;
    }
  tri->id = id;
  tri->level = level;
  shift = 2 * level;
  r = static_cast<htm_root>((id >> shift) & 0x7);
  v0 = *_htm_root_vert[r * 3];
  v1 = *_htm_root_vert[r * 3 + 1];
  v2 = *_htm_root_vert[r * 3 + 2];
  for (shift -= 2; shift >= 0; shift -= 2)
    {
      int child = (id >> shift) & 0x3;
      _htm_vertex (&sv1, &v2, &v0);
      _htm_vertex (&sv2, &v0, &v1);
      _htm_vertex (&sv0, &v1, &v2);
      switch (child)
        {
        case 0:
          v1 = sv2;
          v2 = sv1;
          break;
        case 1:
          v0 = v1;
          v1 = sv0;
          v2 = sv2;
          break;
        case 2:
          v0 = v2;
          v1 = sv1;
          v2 = sv0;
          break;
        case 3:
          v0 = sv0;
          v1 = sv1;
          v2 = sv2;
          break;
        }
    }
  tri->verts[0] = v0;
  tri->verts[1] = v1;
  tri->verts[2] = v2;
  htm_v3_add (&sv0, &v0, &v1);
  htm_v3_add (&sv0, &sv0, &v2);
  htm_v3_normalize (&tri->center, &sv0);
  tri->radius = htm_v3_angsep (&sv0, &v0);
  return HTM_OK;
}

/*  HTM encoding  */

//  probably Decimal encoding
int prob_decimal (ULL in)
{
  int len;
  int dig;
  int pos;
  len = n_dec_digs (in);
  //  printf( "\nprob_decimal() %s\n" , str128( in ) );
  //  ignore first char which may be 0-7
  for (pos = 1; pos < len; pos++)
    {
      dig = dec_dig (in, pos);
      if (3 < dig)
        {
          return FALSE;
        }
    }
  return TRUE;
}

//  definitely Binary encoding
int def_binary (ULL in)
{
  int len;
  int dig;
  int pos;
  len = n_dec_digs (in);
  //  printf( "\ndef_binary() %s\n" , str128( in ) );
  for (pos = 0; pos < len; pos++)
    {
      dig = dec_dig (in, pos);
      if (3 < dig)
        {
          return TRUE;
        }
    }
  return FALSE;
}

//  convert an HTM ID from binary to decimal encoding
int64_t htm_idtodec (int64_t id)
{
  int64_t dec = 0;
  int64_t factor = 1;
  int level = htm_level (id);
  if (level < 0 || level > HTM_DEC_MAX_LEVEL)
    {
      return 0;
    }
  for (++level; level > 0; --level, id >>= 2, factor *= 10)
    {
      dec += factor * (id & 3);
    }
  if ((id & 1) == 1)
    {
      dec += 2 * factor;
    }
  else
    {
      dec += factor;
    }
  return dec;
}

//  alternate convert an HTM ID from binary to decimal encoding
int64_t alt_htm_idtodec (int64_t id)
{
  int64_t dec = 0;
  int64_t factor = 1;
  int level = full_alt_htm_level (id);
  if (level < 0 || level > HTM_DEC_MAX_LEVEL)
    {
      return 0;
    }
  for (++level; level > 0; --level, id >>= 2, factor *= 10)
    {
      dec += factor * (id & 3);
    }
  if ((id & 1) == 1)
    {
      dec += 2 * factor;
    }
  else
    {
      dec += factor;
    }
  return dec;
}

//  convert an HTM ID from decimal to binary encoding
ULL htm_idfrdec (ULL id)
{
  int pow;
  int shc = -2;
  ULL bin = 0;
  int ndig = n_dec_digs (id);
  //  int     level        =     htm_level(  id );
  //  int     level        = alt_htm_level(  id );
  for (pow = ndig - 1; pow >= 0; pow--)
    {
      int slice = dec_dig (id, pow);
      if ((ndig - 1 > pow && 3 < slice) || 0 > slice)
        {
          //  printf( "\n ** illegal Decimal character '%c' @ position %02d
          //  **\n", slice + '0', pow );
          fflush (stdout);
        }
      bin <<= 2;
      shc += 2;
      if (NB <= shc)
        {
          printf ("\n ** shifting beyond %d bits (%d) **\n", NB, shc);
        }
      if (ndig - 1 == pow)
        {
          //  first digit = base 0-7 in 3 bits
          bin |= (ULL)(slice & 7);
          bin += 1;
        }
      else
        {
          bin |= (ULL)(slice & 3);
        }
    }
  return (bin);
}

//  create a string with a base Two depiction of a large number
char *bin_txt (ULL rx)
{
  int vi = TRUE;
  int bp = NB - 1;
  int cp = 0;
  char *tx = static_cast<char *>(malloc (NC));
  memset (tx, EOSC, NC);
  if (vi)
    {
      tx[cp++] = ' ';
      tx[cp++] = ' ';
    }
  while (0 <= bp)
    {
      tx[cp++] = (0 != (rx & ((ULL)1 << bp)) ? '1' : '0');
      if (vi)
        {
          if (0 == bp % SB0)
            {
              tx[cp++] = ' ';
            }
          if (0 == bp % SB1)
            {
              tx[cp++] = ' ';
            }
          if (0 == bp % SB2)
            {
              tx[cp++] = ' ';
            }
        }
      bp--;
    }
  return (tx);
}

//  create a string with a base Ten depiction of a large number
//  static char * str128( U128_TYPE u128 )
//  static char * str128( U64_TYPE u128 ) {
char *str128 (U64_TYPE u128)
{
  char *rtn = (char *)malloc (1024); //  consumer should free() when done
  std::stringstream ss;
  if (0 == rtn)
    {
      ss << "insufficient memory";
    }
  else
    {
      U64_TYPE u64 = u128;
      ss << u64;
    }
  strcpy (rtn, ss.str ().c_str ());
  // FIXME:  Why are we flushing stdout?
  fflush (stdout);
  return (rtn);
}

//  display a large binary number
void sho_bin (int64_t number)
{
  char *bTwo = bin_txt (number);
  char *bTen = str128 (number);
  printf ("\n %s  =  %s\n", bTwo, bTen);
  free (bTwo);
  free (bTen);
}

//  return binary value of a selected decimal digit from the base ten
//  representation of an int
int dec_dig (int64_t num, int pow)
{
  char buf[64];
  strcpy (buf, str128 (num));
  int pos = strlen (buf);
  if (0 > pos)
    return (-1);
  int dig = (char)buf[pos - pow - 1] - '0';
  return (dig);
}

//  count number of digits in base ten representation
int n_dec_digs (int64_t num) { return (strlen (str128 (num))); }

int64_t htm_tree_s2circle_count (const struct htm_tree *tree,
                                 const struct htm_v3 *center, double radius,
                                 enum htm_errcode *err)
{
  return htm_tree_s2circle (tree, center, radius, err, NULL);
}

int64_t htm_tree_s2ellipse_count (const struct htm_tree *tree,
                                  const struct htm_s2ellipse *ellipse,
                                  enum htm_errcode *err)
{
  return htm_tree_s2ellipse (tree, ellipse, err, NULL);
}

int64_t htm_tree_s2cpoly_count (const struct htm_tree *tree,
                                const struct htm_s2cpoly *poly,
                                enum htm_errcode *err)
{
  return htm_tree_s2cpoly (tree, poly, err, NULL);
}
}
