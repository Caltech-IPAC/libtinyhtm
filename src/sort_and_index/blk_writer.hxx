/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#ifndef HTM_TREE_GEN_BLK_WRITER_H
#define HTM_TREE_GEN_BLK_WRITER_H

/* ---- Asynchronous block writer ---- */

#include <algorithm>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "blk_write_state.hxx"

template <class T> void blk_writer_issue (T &b) { b.issue_helper (); }

void *blk_write (void *arg);

template <class T, bool sorted = true> struct blk_writer
{
  size_t bytes_per_block; /* Number of bytes per block */
  size_t bytes_in_block;  /* Number of bytes in current block */
  unsigned char *buf;     /* Current block pointer */
  void *mem;              /* Space for 2 blocks of n tree entries */
  size_t memsz;           /* Size of mem in bytes */
  int fd;                 /* File descriptor of file being written to */
  enum blk_write_state state;
  unsigned char *wrbuf;
  size_t wrbytes;
  pthread_attr_t attr;
  pthread_mutex_t mtx;
  pthread_cond_t cv;
  pthread_t thr;

  blk_writer (const std::string &file, size_t blksz)
      : bytes_per_block (blksz), bytes_in_block (0),
        fd (open (file.c_str (), O_CREAT | O_TRUNC | O_APPEND | O_WRONLY,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)),
        state (BK_WRITE_START), wrbuf (nullptr), wrbytes (0)
  {
    if (fd == -1)
      throw std::runtime_error ("failed to open file " + file
                                + " for writing");

    memsz = 2 * blksz;
    const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
    if (memsz % pagesz != 0)
      {
        memsz += pagesz - memsz % pagesz;
      }
    mem = std::calloc (1, memsz);
    if (mem == NULL)
      throw std::runtime_error ("write buffer allocation calloc() failed when "
                                "allocating " + std::to_string (memsz)
                                + " bytes.");
    buf = (unsigned char *)mem;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init (&mtx, NULL);
    pthread_cond_init (&cv, NULL);
    pthread_create (&thr, &attr, &blk_write, (void *)this);
  }

  void append (const void *const data) { append (data, sizeof(T)); }

  void append (const void *const data, const size_t nbytes)
  {
    size_t i = bytes_in_block;
    if (i + nbytes > bytes_per_block)
      {
        issue ();
        i = 0;
      }
    assert (i + nbytes <= bytes_per_block);
    memcpy (buf + i, data, nbytes);
    bytes_in_block = i + nbytes;
  }

  void issue () { blk_writer_issue (*this); }

  /*  Issues (asynchronous) write of one block. */
  void issue_helper ()
  {
    pthread_mutex_lock (&mtx);
    /* wait until background writer thread is ready for a command */
    while (state != BK_WRITE_READY && state != BK_WRITE_ERR)
      {
        pthread_cond_wait (&cv, &mtx);
      }
    if (state == BK_WRITE_ERR)
      {
        pthread_mutex_unlock (&mtx);
        throw std::runtime_error (
            "background thread failed to write() disk block");
      }
    /* issue write for the current block */
    state = BK_WRITE_BLK;
    wrbuf = buf;
    wrbytes = bytes_in_block;
    pthread_cond_signal (&cv);
    pthread_mutex_unlock (&mtx);
    /* flip write buffer */
    bytes_in_block = 0;
    if (buf == (unsigned char *)mem)
      {
        buf = ((unsigned char *)mem) + bytes_per_block;
      }
    else
      {
        buf = ((unsigned char *)mem);
      }
  }

  /*  Closes writer w; all as yet unwritten data is flushed to disk
      first. */
  void close ()
  {
    issue ();
    pthread_mutex_lock (&mtx);
    /* wait until background writer thread is ready for a command */
    while (state != BK_WRITE_READY && state != BK_WRITE_ERR)
      {
        pthread_cond_wait (&cv, &mtx);
      }
    if (state == BK_WRITE_ERR)
      {
        pthread_mutex_unlock (&mtx);
        throw std::runtime_error (
            "background thread failed to write() disk block");
      }
    /* issue thread exit command ... */
    state = BK_WRITE_EXIT;
    pthread_cond_signal (&cv);
    pthread_mutex_unlock (&mtx);
    /* ... and wait for writer thread to terminate */
    pthread_join (thr, NULL);
    /* clean up */
    free (mem);

    if (::close (fd) != 0)
      throw std::runtime_error ("file close() failed");
    pthread_cond_destroy (&cv);
    pthread_mutex_destroy (&mtx);
    pthread_attr_destroy (&attr);
  }

  ~blk_writer () { close (); }
  blk_writer () = delete;
};

template <class T> void blk_writer_issue (blk_writer<T, true> &b)
{
  std::sort (reinterpret_cast<T *>(b.buf),
             reinterpret_cast<T *>(b.buf + b.bytes_in_block));
  b.issue_helper ();
}

#endif
