/*  -------------------------------------------------------------------------
    Copyright (C) 2012-2014 Intel Mobile Communications GmbH

         Sec Class: Intel Confidential (IC)

     ----------------------------------------------------------------------
     Revision Information:
       $$File name:  /msw_tools/FlashTool/libFlsTool/src/c_sha1.h $
     ---------------------------------------------------------------------- */

/* This code is adapted from free code written by Paul E. Jones:

  Copyright (C) 1998, 2009
  Paul E. Jones <paulej@packetizer.com>

  Freeware Public License (FPL)

  This software is licensed as "freeware."  Permission to distribute
  this software in source and binary forms, including incorporation
  into other products, is hereby granted without a fee.  THIS SOFTWARE
  IS PROVIDED 'AS IS' AND WITHOUT ANY EXPRESSED OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS FOR A PARTICULAR PURPOSE.  THE AUTHOR SHALL NOT BE HELD
  LIABLE FOR ANY DAMAGES RESULTING FROM THE USE OF THIS SOFTWARE, EITHER
  DIRECTLY OR INDIRECTLY, INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA
  OR DATA BEING RENDERED INACCURATE.
*/

#ifndef _C_SHA1_H_
#define _C_SHA1_H_

#define MESSAGE_DIGEST_LENGTH   20
#define uint32 unsigned long
/*
 *  This structure will hold context information for the hashing
 *  operation
 */
typedef struct
{
    uint32 Message_Digest[5];   /* Message Digest (output)          */

    uint32 Length_Low;          /* Message length in bits           */
    uint32 Length_High;         /* Message length in bits           */

    unsigned char Message_Block[64];/* 512-bit message blocks      */
    int Message_Block_Index;      /* Index into message block array   */

    int Computed;                 /* Is the digest computed?          */
    int Corrupted;                /* Is the message digest corruped?  */
} SHA1Context;

/*
 *  Function Prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

void SHA1Reset(SHA1Context *);
int SHA1Result(SHA1Context *);
void SHA1Input( SHA1Context *,
                const unsigned char *,
                uint32);

#ifdef __cplusplus
}
#endif

#endif
