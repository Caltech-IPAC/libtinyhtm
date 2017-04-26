/*  Grows hash table capacity by a factor of two.
 */

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "../hash_table.hxx"

void hash_table_grow (struct hash_table *ht)
{
  size_t i, cap = ht->cap;
  struct id_off **array = ht->array;

  ht->cap = 2 * cap;
  ht->array = (struct id_off **)malloc (ht->cap * sizeof(struct id_off *));
  if (ht->array == NULL)
    {
      throw std::runtime_error ("malloc() failed");
    }
  memset (ht->array, 0, ht->cap * sizeof(struct id_off *));

  /* add previous hash table entries */
  for (i = 0; i < cap; ++i)
    {
      struct id_off *e = array[i];
      while (e != NULL)
        {
          struct id_off *next = e->next;
          size_t j = (2 * cap - 1) & (size_t)e->id.block[NLOD];
          e->next = ht->array[j];
          ht->array[j] = e;
          e = next;
        }
    }
}
