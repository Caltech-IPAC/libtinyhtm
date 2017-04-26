#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "../hash_table.hxx"

void hash_table_init (struct hash_table *ht)
{
  ht->cap = 65536; /* must be a power of 2 */
  ht->n = 0;
  ht->array = (struct id_off **)malloc (ht->cap * sizeof(struct id_off *));
  if (ht->array == NULL)
    {
      throw std::runtime_error ("malloc() failed");
    }
  memset (ht->array, 0, ht->cap * sizeof(struct id_off *));
#if FAST_ALLOC
  ht->ar = new arena (sizeof(struct id_off));
#endif
}
