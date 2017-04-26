#ifndef HTM_TREE_GEN_ASSIGN_BLOCK_H
#define HTM_TREE_GEN_ASSIGN_BLOCK_H

/*  Assigns a block ID at the specified level-of-detail to all nodes in
    the sub-tree rooted at n that do not already have a block ID at that
    level-of-detail. When assigning a block ID to a node, check whether all
    block IDs are now valid and if so write the node out to disk.
    Children of nodes that are written are destroyed.
 */

#include <cstdint>
#include "../tree_gen_context.hxx"
#include "../node.hxx"

void assign_block (tree_gen_context &ctx, mem_node *const n,
                   const uint64_t blockid, const int lod);

#endif
