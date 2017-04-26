/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#include <unistd.h>
#include <cerrno>
#include "../blk_writer.hxx"

/*  Writes data blocks in the background.
 */
void *blk_write (void *arg)
{
  /* We do not call any of the member functions of the blk_writer, so
     it does not actually matter what the exact templated type is.  So
     we use char. */
  blk_writer<unsigned char, false> *w (
      static_cast<blk_writer<unsigned char, false> *>(arg));
  pthread_mutex_lock (&w->mtx);
  while (1)
    {
      unsigned char *buf;
      size_t n;
      ssize_t b;
      /* signal readiness for a command */
      w->state = BK_WRITE_READY;
      pthread_cond_signal (&w->cv);
      /* wait for a command */
      pthread_cond_wait (&w->cv, &w->mtx);
      if (w->state == BK_WRITE_READY)
        {
          continue; /* nothing to do */
        }
      else if (w->state == BK_WRITE_EXIT)
        {
          break; /* exit background writer thread */
        }
      /* write a block */
      buf = w->wrbuf;
      n = w->wrbytes;
      pthread_mutex_unlock (&w->mtx);
      while (n > 0)
        {
          b = write (w->fd, buf, n);
          if (b < 0)
            {
              if (errno != EINTR)
                {
                  /* error - exit background writer thread */
                  pthread_mutex_lock (&w->mtx);
                  w->state = BK_WRITE_ERR;
                  goto done;
                }
            }
          else
            {
              buf += b;
              n -= (size_t)b;
            }
        }
      pthread_mutex_lock (&w->mtx);
    }
done:
  pthread_cond_signal (&w->cv);
  pthread_mutex_unlock (&w->mtx);
  return NULL;
}
