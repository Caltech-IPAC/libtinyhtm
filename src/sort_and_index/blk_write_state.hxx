/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#ifndef HTM_TREE_GEN_BLK_WRITE_STATE_H
#define HTM_TREE_GEN_BLK_WRITE_STATE_H

enum blk_write_state
{
  BK_WRITE_START = 0,
  BK_WRITE_READY,
  BK_WRITE_BLK,
  BK_WRITE_EXIT,
  BK_WRITE_ERR
};

#endif
