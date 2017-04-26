#pragma once

#include "tinyhtm/htm.h"

/*
    HTM triangles are subdivided into 4 sub-triangles as follows :

            v2
             *
            / \
           /   \
      sv1 *-----* sv0
         / \   / \
        /   \ /   \
    v0 *-----*-----* v1
            sv2

     -  vertices are unit magnitude 3-vectors
     -  edges are great circles on the unit sphere
     -  vertices are stored in counter-clockwise order
       (when viewed from outside the unit sphere in a
       right handed coordinate system)
     -  sv0 = (v1 + v2) / ||v1 + v2||, and likewise for sv1, sv2

    Note that if the HTM triangle given by (v0,v1,v2) has index I, then:
     -  sub triangle T0 = (v0,sv2,sv1) has index I*4
     -  sub triangle T1 = (v1,sv0,sv2) has index I*4 + 1
     -  sub triangle T2 = (v2,sv1,sv0) has index I*4 + 2
     -  sub triangle T3 = (sv0,sv1,sv2) has index I*4 + 3

    All HTM triangles are obtained via subdivision of 8 initial
    triangles, defined from the following set of 6 vertices :
     -  V0 = ( 0,  0,  1) north pole
     -  V1 = ( 1,  0,  0)
     -  V2 = ( 0,  1,  0)
     -  V3 = (-1,  0,  0)
     -  V4 = ( 0, -1,  0)
     -  V5 = ( 0,  0, -1) south pole

    The root triangles (corresponding to subdivision level 0) are :
     -  S0 = (V1, V5, V2), HTM index = 8
     -  S1 = (V2, V5, V3), HTM index = 9
     -  S2 = (V3, V5, V4), HTM index = 10
     -  S3 = (V4, V5, V1), HTM index = 11
     -  N0 = (V1, V0, V4), HTM index = 12
     -  N1 = (V4, V0, V3), HTM index = 13
     -  N2 = (V3, V0, V2), HTM index = 14
     -  N3 = (V2, V0, V1), HTM index = 15

    'S' denotes a triangle in the southern hemisphere,
    'N' denotes a triangle in the northern hemisphere.
 */

// #ifdef __cplusplus
// extern "C" {
// #endif

/* ---- Types ---- */

/** HTM triangle vs. region classification codes.
  */
enum _htm_cov
{
  HTM_DISJOINT = 0,  /**< HTM triangle disjoint from region. */
  HTM_INTERSECT = 1, /**< HTM triangle intersects region. */
  HTM_CONTAINS = 2,  /**< HTM triangle completely contains region. */
  HTM_INSIDE = 3     /**< HTM triangle completely inside region. */
};

/** A node (triangle/trixel) in an HTM tree.
  */
struct _htm_node
{
  struct htm_v3 mid_vert[3];    /**< Triangle edge mid-points. */
  struct htm_v3 mid_edge[3];    /**< Subdivision plane normals. */
  const struct htm_v3 *vert[3]; /**< Triangle vertex pointers. */
  const struct htm_v3 *edge[3]; /**< Triangle edge normal pointers. */
  struct htm_v3p *end;          /**< Temporary used for HTM indexing. */
  const unsigned char *s;       /**< Temporary used for tree searches. */
  uint64_t index;               /**< Temporary used for tree searches. */
  int64_t id;                   /**< HTM ID of the node. */
  int child;                    /**< Index of next child (0-3). */
};

/** A root to leaf path in a depth-first traversal of an HTM tree.
  */
struct _htm_path
{
  enum htm_root root; /**< ordinal of root triangle (0-7) */
  struct _htm_node node[HTM_MAX_LEVEL + 1];
};

/* ---- Data ---- */

/*  HTM root triangle vertices/edge plane normals.
 */
static const struct htm_v3 _htm_root_v3[6] = { { 0.0, 0.0, 1.0 },
                                               { 1.0, 0.0, 0.0 },
                                               { 0.0, 1.0, 0.0 },
                                               { -1.0, 0.0, 0.0 },
                                               { 0.0, -1.0, 0.0 },
                                               { 0.0, 0.0, -1.0 } };

#define HTM_Z &_htm_root_v3[0]
#define HTM_X &_htm_root_v3[1]
#define HTM_Y &_htm_root_v3[2]
#define HTM_NX &_htm_root_v3[3]
#define HTM_NY &_htm_root_v3[4]
#define HTM_NZ &_htm_root_v3[5]

/*  Vertex pointers for the 3 vertices of each HTM root triangle.
 */
static const struct htm_v3 *const _htm_root_vert[24] = {
  HTM_X,  HTM_NZ, HTM_Y,  /* S0 */
  HTM_Y,  HTM_NZ, HTM_NX, /* S1 */
  HTM_NX, HTM_NZ, HTM_NY, /* S2 */
  HTM_NY, HTM_NZ, HTM_X,  /* S3 */
  HTM_X,  HTM_Z,  HTM_NY, /* N0 */
  HTM_NY, HTM_Z,  HTM_NX, /* N1 */
  HTM_NX, HTM_Z,  HTM_Y,  /* N2 */
  HTM_Y,  HTM_Z,  HTM_X   /* N3 */
};

/*  Edge normal pointers for the 3 edge normals of each HTM root triangle.
 */
static const struct htm_v3 *const _htm_root_edge[24] = {
  HTM_Y,  HTM_X,  HTM_NZ, /* S0 */
  HTM_NX, HTM_Y,  HTM_NZ, /* S1 */
  HTM_NY, HTM_NX, HTM_NZ, /* S2 */
  HTM_X,  HTM_NY, HTM_NZ, /* S3 */
  HTM_NY, HTM_X,  HTM_Z,  /* N0 */
  HTM_NX, HTM_NY, HTM_Z,  /* N1 */
  HTM_Y,  HTM_NX, HTM_Z,  /* N2 */
  HTM_X,  HTM_Y,  HTM_Z   /* N3 */
};

/* ---- Implementation details ---- */

/*  Sets path to the i-th HTM root triangle.
 */
HTM_INLINE void _htm_path_root (struct _htm_path *path, enum htm_root r)
{
  path->node[0].vert[0] = _htm_root_vert[r * 3];
  path->node[0].vert[1] = _htm_root_vert[r * 3 + 1];
  path->node[0].vert[2] = _htm_root_vert[r * 3 + 2];
  path->node[0].edge[0] = _htm_root_edge[r * 3];
  path->node[0].edge[1] = _htm_root_edge[r * 3 + 1];
  path->node[0].edge[2] = _htm_root_edge[r * 3 + 2];
  path->node[0].id = r + 8;
  path->node[0].child = 0;
  path->root = r;
}

//  /*  Computes the normalized average of two input vertices.
//   */
//  HTM_INLINE void _htm_vertex(struct htm_v3 *out,
//                              const struct htm_v3 *v1,
//                              const struct htm_v3 *v2)
//  {
//      htm_v3_add(out, v1, v2);
//      htm_v3_normalize(out, out);
//  }

/*  Computes quantities needed by _htm_node_make0(node).
 */
HTM_INLINE void _htm_node_prep0 (struct _htm_node *node)
{
  _htm_vertex (&node->mid_vert[1], node->vert[2], node->vert[0]);
  _htm_vertex (&node->mid_vert[2], node->vert[0], node->vert[1]);
  htm_v3_rcross (&node->mid_edge[1], &node->mid_vert[2], &node->mid_vert[1]);
}

/*  Sets node[1] to child 0 of node[0]. Assumes _htm_node_prep0(node)
    has been called.
 */
HTM_INLINE void _htm_node_make0 (struct _htm_node *node)
{
  node[1].vert[0] = node[0].vert[0];
  node[1].vert[1] = &node[0].mid_vert[2];
  node[1].vert[2] = &node[0].mid_vert[1];
  node[1].edge[0] = node[0].edge[0];
  node[1].edge[1] = &node[0].mid_edge[1];
  node[1].edge[2] = node[0].edge[2];
  node[0].child = 1;
  node[1].id = node[0].id << 2;
  node[1].child = 0;
}

/*  Computes quantities needed by _htm_node_make1(node). Assumes
    _htm_node_prep0(node) has been called.
 */
HTM_INLINE void _htm_node_prep1 (struct _htm_node *node)
{
  _htm_vertex (&node->mid_vert[0], node->vert[1], node->vert[2]);
  htm_v3_rcross (&node->mid_edge[2], &node->mid_vert[0], &node->mid_vert[2]);
}

/*  Sets node[1] to child 1 of node[0]. Assumes _htm_node_prep1(node)
    has been called.
 */
HTM_INLINE void _htm_node_make1 (struct _htm_node *node)
{
  node[1].vert[0] = node[0].vert[1];
  node[1].vert[1] = &node[0].mid_vert[0];
  node[1].vert[2] = &node[0].mid_vert[2];
  node[1].edge[0] = node[0].edge[1];
  node[1].edge[1] = &node[0].mid_edge[2];
  node[1].edge[2] = node[0].edge[0];
  node[0].child = 2;
  node[1].id = (node[0].id << 2) + 1;
  node[1].child = 0;
}

/*  Computes quantities needed by _htm_node_make2(node). Assumes
    _htm_node_prep1 has been called.
 */
HTM_INLINE void _htm_node_prep2 (struct _htm_node *node)
{
  htm_v3_rcross (&node->mid_edge[0], &node->mid_vert[1], &node->mid_vert[0]);
}

/*  Sets node[1] to child 2 of node[0]. Assumes _htm_node_prep2(node)
    has been called.
 */
HTM_INLINE void _htm_node_make2 (struct _htm_node *node)
{
  node[1].vert[0] = node[0].vert[2];
  node[1].vert[1] = &node[0].mid_vert[1];
  node[1].vert[2] = &node[0].mid_vert[0];
  node[1].edge[0] = node[0].edge[2];
  node[1].edge[1] = &node[0].mid_edge[0];
  node[1].edge[2] = node[0].edge[1];
  node[0].child = 3;
  node[1].id = (node[0].id << 2) + 2;
  node[1].child = 0;
}

/*  Sets node[1] to child 3 of node[0]. Assumes _htm_node_prep2(node)
    has been called.
 */
HTM_INLINE void _htm_node_make3 (struct _htm_node *node)
{
  htm_v3_neg (&node[0].mid_edge[0], &node[0].mid_edge[0]);
  htm_v3_neg (&node[0].mid_edge[1], &node[0].mid_edge[1]);
  htm_v3_neg (&node[0].mid_edge[2], &node[0].mid_edge[2]);
  node[1].vert[0] = &node[0].mid_vert[0];
  node[1].vert[1] = &node[0].mid_vert[1];
  node[1].vert[2] = &node[0].mid_vert[2];
  node[1].edge[0] = &node[0].mid_edge[0];
  node[1].edge[1] = &node[0].mid_edge[1];
  node[1].edge[2] = &node[0].mid_edge[2];
  node[0].child = 4;
  node[1].id = (node[0].id << 2) + 3;
  node[1].child = 0;
}

// #ifdef __cplusplus
// }
// #endif
