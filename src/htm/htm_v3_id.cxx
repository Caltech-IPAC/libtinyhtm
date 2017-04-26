#include "htm.hxx"
#include "htm/_htm_v3_htmroot.hxx"

extern "C" {

int64_t htm_v3_id (const struct htm_v3 *point, int level)
{
  struct htm_v3 v0, v1, v2;
  struct htm_v3 sv0, sv1, sv2;
  struct htm_v3 e;
  int64_t id;
  int curlevel;
  enum htm_root r;

  if (point == NULL)
    {
      return 0;
    }
  if (level < 0 || level > HTM_MAX_LEVEL)
    {
      return 0;
    }
  r = _htm_v3_htmroot (point);
  v0 = *_htm_root_vert[r * 3];
  v1 = *_htm_root_vert[r * 3 + 1];
  v2 = *_htm_root_vert[r * 3 + 2];
  id = r + 8;
  for (curlevel = 0; curlevel < level; ++curlevel)
    {
      _htm_vertex (&sv1, &v2, &v0);
      _htm_vertex (&sv2, &v0, &v1);
      htm_v3_rcross (&e, &sv2, &sv1);
      if (htm_v3_dot (&e, point) >= 0)
        {
          v1 = sv2;
          v2 = sv1;
          id = id << 2;
          continue;
        }
      _htm_vertex (&sv0, &v1, &v2);
      htm_v3_rcross (&e, &sv0, &sv2);
      if (htm_v3_dot (&e, point) >= 0)
        {
          v0 = v1;
          v1 = sv0;
          v2 = sv2;
          id = (id << 2) + 1;
          continue;
        }
      htm_v3_rcross (&e, &sv1, &sv0);
      if (htm_v3_dot (&e, point) >= 0)
        {
          v0 = v2;
          v1 = sv1;
          v2 = sv0;
          id = (id << 2) + 2;
        }
      else
        {
          v0 = sv0;
          v1 = sv1;
          v2 = sv2;
          id = (id << 2) + 3;
        }
    }
  return id;
}
}
