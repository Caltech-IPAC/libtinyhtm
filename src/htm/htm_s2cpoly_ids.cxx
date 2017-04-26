#include "htm/_htm_ids_init.hxx"
#include "_htm_ids_add.hxx"
#include "_htm_simplify_ids.hxx"
#include "_htm_s2cpoly_htmcov.hxx"

extern "C" {

struct htm_ids *htm_s2cpoly_ids (struct htm_ids *ids,
                                 const struct htm_s2cpoly *poly, int level,
                                 size_t maxranges, enum htm_errcode *err)
{
  double stackab[2 * 256 + 4];
  struct _htm_path path;
  double *ab;
  size_t nb;
  int efflevel;

  if (poly == NULL)
    {
      if (err != NULL)
        {
          *err = HTM_ENULLPTR;
        }
      free (ids);
      return NULL;
    }
  else if (level < 0 || level > HTM_MAX_LEVEL)
    {
      if (err != NULL)
        {
          *err = HTM_ELEVEL;
        }
      free (ids);
      return NULL;
    }
  if (ids == NULL)
    {
      ids = _htm_ids_init ();
      if (ids == NULL)
        {
          if (err != NULL)
            {
              *err = HTM_ENOMEM;
            }
          return NULL;
        }
    }
  else
    {
      ids->n = 0;
    }
  nb = (2 * poly->n + 4) * sizeof(double);
  if (nb > sizeof(stackab))
    {
      ab = (double *)malloc (nb);
      if (ab == NULL)
        {
          if (err != NULL)
            {
              *err = HTM_ENOMEM;
            }
          free (ids);
          return NULL;
        }
    }
  else
    {
      ab = stackab;
    }

  efflevel = level;

  for (int root = HTM_S0; root <= HTM_N3; ++root)
    {
      struct _htm_node *curnode = path.node;
      int curlevel = 0;
      _htm_path_root (&path, static_cast<htm_root>(root));

      while (1)
        {
          switch (_htm_s2cpoly_htmcov (curnode, poly, ab))
            {
            case HTM_CONTAINS:
              if (curlevel == 0)
                {
                  /* no need to consider other roots */
                  root = HTM_N3;
                }
              else
                {
                  /* no need to consider other children of parent */
                  curnode[-1].child = 4;
                }
            /* fall-through */
            case HTM_INTERSECT:
              if (curlevel < efflevel)
                {
                  /* continue subdividing */
                  _htm_node_prep0 (curnode);
                  _htm_node_make0 (curnode);
                  ++curnode;
                  ++curlevel;
                  continue;
                }
            /* fall-through */
            case HTM_INSIDE:
              /* reached a leaf or fully covered HTM triangle,
                 append HTM ID range to results */
              {
                int64_t id = curnode->id << (level - curlevel) * 2;
                int64_t n = ((int64_t)1) << (level - curlevel) * 2;
                ids = _htm_ids_add (ids, id, id + n - 1);
              }
              if (ids == NULL)
                {
                  if (ab != stackab)
                    {
                      free (ab);
                    }
                  if (err != NULL)
                    {
                      *err = HTM_ENOMEM;
                    }
                  return ids;
                }
              while (ids->n > maxranges && efflevel != 0)
                {
                  /* too many ranges:
                     reduce effetive subdivision level */
                  --efflevel;
                  if (curlevel > efflevel)
                    {
                      curnode = curnode - (curlevel - efflevel);
                      curlevel = efflevel;
                    }
                  _htm_simplify_ids (ids, level - efflevel);
                }
              break;
            default:
              /* HTM triangle does not intersect polygon */
              break;
            }
          /* ascend towards the root */
          --curlevel;
          --curnode;
          while (curlevel >= 0 && curnode->child == 4)
            {
              --curnode;
              --curlevel;
            }
          if (curlevel < 0)
            {
              /* finished with this root */
              break;
            }
          if (curnode->child == 1)
            {
              _htm_node_prep1 (curnode);
              _htm_node_make1 (curnode);
            }
          else if (curnode->child == 2)
            {
              _htm_node_prep2 (curnode);
              _htm_node_make2 (curnode);
            }
          else
            {
              _htm_node_make3 (curnode);
            }
          ++curnode;
          ++curlevel;
        }
    }
  if (ab != stackab)
    {
      free (ab);
    }
  if (err != NULL)
    {
      *err = HTM_OK;
    }
  return ids;
}
}
