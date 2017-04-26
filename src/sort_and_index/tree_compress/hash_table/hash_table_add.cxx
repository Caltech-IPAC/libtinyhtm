/*  Adds an id to offset mapping to the given hash table.
 */

#include <stdexcept>
#include "../hash_table.hxx"

void hash_table_add (struct hash_table *const ht,
                     const struct node_id *const id, uint64_t off)
{
  struct id_off *e;
  size_t i;
  if ((ht->n * 3) / 4 > ht->cap)
    {
      hash_table_grow (ht);
    }
  i = (ht->cap - 1) & (size_t)id->block[NLOD];
#if FAST_ALLOC
  e = (struct id_off *)ht->ar->alloc ();
#else
  e = (struct id_off *)malloc (sizeof(struct id_off));
  if (e == NULL)
    {
      throw std::runtime_error ("malloc() failed");
    }
#endif
  e->id = *id;
  e->off = off;
  e->next = ht->array[i];
  ht->array[i] = e;
  ++ht->n;
}
