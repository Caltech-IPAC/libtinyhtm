#include <cstdlib>
#include "../hash_table.hxx"

void hash_table_destroy (struct hash_table *ht)
{
#if FAST_ALLOC
  delete (ht->ar);
#else
  size_t i;
  struct id_off *ptr;
  for (i = 0; i < ht->cap; ++i)
    {
      ptr = ht->array[i];
      while (ptr != NULL)
        {
          struct id_off *next = ptr->next;
          free (ptr);
          ptr = next;
        }
    }
#endif
  free (ht->array);
  ht->array = NULL;
  ht->n = 0;
  ht->cap = 0;
}
