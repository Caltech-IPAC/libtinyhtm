#include <sys/time.h>
#include <cstddef>

/*  Returns the number of seconds that have elapsed since the epoch.
 */
double now ()
{
  struct timeval t;
  gettimeofday (&t, NULL);
  return ((double)t.tv_sec) + ((double)t.tv_usec) / 1.0e6;
}
