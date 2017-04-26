/*  Returns the offset of the node with the given ID and removes
    the corresponding hash table entry.
 */

#include <stdexcept>
#include "../hash_table.hxx"

uint64_t hash_table_get (struct hash_table *const ht,
                         const struct node_id *const id)
{
  struct id_off *e, *prev;
  const size_t i = (ht->cap - 1) & (size_t)id->block[NLOD];
  prev = NULL;
  e = ht->array[i];
  /* child must occur before parent */
  while (1)
    {
      if (e == NULL)
        {
          throw std::runtime_error (
              "tree generation bug: parent node written before child");
        }
      if (node_id_eq (id, &e->id))
        {
          uint64_t off = e->off;
          if (prev == NULL)
            {
              ht->array[i] = e->next;
            }
          else
            {
              prev->next = e->next;
            }
#if FAST_ALLOC
          ht->ar->free (e);
#else
          free (e);
#endif
          --ht->n;
          return off;
        }
      prev = e;
      e = e->next;
    }
  /* never reached */
}
