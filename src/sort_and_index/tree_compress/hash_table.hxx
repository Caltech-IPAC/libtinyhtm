/*  Simple hash table that maps node IDs to relative file offsets. The
    implementation uses a power-of-2 sized backing array and chains on
    collision. The hash-code of a node ID is taken to be its post-order
    index (which is unique).
 */

#ifndef HTM_TREE_GEN_HASH_TABLE_H
#define HTM_TREE_GEN_HASH_TABLE_H

#include "id_off.hxx"
#include "../arena.hxx"

struct hash_table
{
  size_t n;
  size_t cap;
  struct id_off **array;
#if FAST_ALLOC
  struct arena *ar;
#endif
};

void hash_table_init (struct hash_table *ht);
void hash_table_destroy (struct hash_table *ht);
void hash_table_grow (struct hash_table *ht);
uint64_t hash_table_get (struct hash_table *const ht,
                         const struct node_id *const id);
void hash_table_add (struct hash_table *const ht,
                     const struct node_id *const id, uint64_t off);

#endif
