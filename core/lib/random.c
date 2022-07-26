/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * @(#)$Id: random.c,v 1.2 2008/02/10 12:30:57 oliverschmidt Exp $
 */

/*
 * Minimal standard random number generator.
 *
 * Simple crappy ANSI C compatible random number generator that is
 * good enough for us!
 *
 * Park, S.K. and K.W. Miller, 1988;
 * Random Number Generators: Good Ones are Hard to Find,
 * Comm. of the ACM, V. 31. No. 10, pp 1192-1201
 */


#include "lib/random.h"
#include "sys/clock.h"

static uint32_t rand_state = 1;

void
random_init(unsigned short seed)
{
  rand_state = seed;
}

unsigned short
random_rand(void)
{
  uint32_t hi, lo;

  /*
   * Perform two 16x16 bits multiplication with 32 bit results.
   */
  lo = 16807ul * (uint16_t)(rand_state);
  hi = 16807ul * (uint16_t)(rand_state >> 16);

  lo += (hi & 0x7fff) << 16;

  /*
   * lo += hi >> 15; But faster using 16 bit registers.
   */
  hi <<= 1;
  lo += (uint16_t)(hi >> 16);

  if ((int32_t)lo <= 0)		/* Deal with rand_state == 0. */
    lo -= 0x7fffffff;

  rand_state = lo;

  if (sizeof(int) == sizeof(int16_t))
    return (lo ^ (lo >> 16)) & RANDOM_RAND_MAX; /* Not ANSI C! */
  else
    return lo & RANDOM_RAND_MAX;

}
