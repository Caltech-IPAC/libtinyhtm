#ifndef HTM_TREE_GEN_EMIT_NODE_H
#define HTM_TREE_GEN_EMIT_NODE_H

/*  Called on a node when all points belonging to it have been accounted for.
 */

#include "../tree_gen_context.hxx"
#include "../node.hxx"

void emit_node (mem_node *const node, tree_gen_context &ctx);

#endif
