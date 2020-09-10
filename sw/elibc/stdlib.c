// =============================================================================
//  Program : stdlib.c
//  Author  : Chun-Jen Tsai
//  Date    : Dec/09/2019
// -----------------------------------------------------------------------------
//  Description:
//  This is the minimal stdlib library for Aquila.
// -----------------------------------------------------------------------------
//  Revision information:
//
//  None.
// -----------------------------------------------------------------------------
//  License information:
//
//  This software is released under the BSD-3-Clause Licence,
//  see https://opensource.org/licenses/BSD-3-Clause for details.
//  In the following license statements, "software" refers to the
//  "source code" of the complete hardware/software system.
//
//  Copyright 2019,
//                    Embedded Intelligent Systems Lab (EISL)
//                    Deparment of Computer Science
//                    National Chiao Tung Uniersity
//                    Hsinchu, Taiwan.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// =============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long ulong;

extern ulong __heap_start; /* declared in the linker script */
extern ulong __heap_size;  /* declared in the linker script */
static ulong heap_top = (ulong) &__heap_start;
static ulong heap_size = (ulong) &__heap_size;
static ulong *curr_top = (ulong *) 0xFFFFFFF0, *heap_end = (ulong *) 0xFFFFFFF0;

void *malloc(size_t n)
{
    void *return_ptr;
    ulong *ptr, temp;
    int r;

    if (curr_top == heap_end) // first time to call malloc()?
    {
        curr_top = (ulong *) heap_top;
        heap_end = (ulong *) ((heap_top + heap_size) & 0xFFFFFFF0);
        *curr_top = (ulong) heap_end;
    }

    // Search for a large-enough free memory block (FMB).
    return_ptr = NULL;
    for (ptr = curr_top; ptr < heap_end; ptr = (ulong *) *ptr)
    {
        if ((*ptr & 1) == 0 && (*ptr - (ulong) ptr > n))
        {
            temp = ((ulong) ptr) & 0xFFFFFFFE;
            return_ptr = (void *) (temp + sizeof(ulong));

            // Update the FMB link list structure.
            r = n % sizeof(ulong);
            temp = n + sizeof(ulong) + ((r)? 4-r : 0);
            *(ptr + temp/sizeof(ulong)) = *ptr;
            curr_top = ptr + temp/sizeof(ulong);
            *ptr = (*ptr + temp) | 1;
            break;
        }
    }

    if (return_ptr != NULL) return return_ptr;

    // Search again for a FMB from heap_top to curr_top
    for (ptr = (ulong *) heap_top; ptr < curr_top; ptr = (ulong *) *ptr)
    {
        if ((*ptr & 1) == 0 && (*ptr - (ulong) ptr > n))
        {
            temp = ((ulong) ptr) & 0xFFFFFFFE;
            return_ptr = (void *) (temp + sizeof(ulong));

            // Update the FMB link list structure.
            r = n % sizeof(ulong);
            temp = n + sizeof(ulong) + ((r)? 4-r : 0);
            *(ptr + temp/sizeof(ulong)) = *ptr;
            curr_top = ptr + temp/sizeof(ulong);
            *ptr = (*ptr + temp) | 1;
            break;
        }
    }

    return return_ptr;
}

void free(void *mptr)
{
    ulong *ptr;

    ptr = ((ulong *) mptr) - 1;
    *ptr = *ptr & 0xFFFFFFFE; // Free the FMB.
}

void *calloc(size_t n, size_t size)
{
    void *mptr;
    mptr = malloc(n*size);
    memset(mptr, 0, n*size);
    return mptr;
}

int atoi(char *s)
{
    int value, sign;

    /* skip leading while characters */
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') sign = -1, s++;
    else sign = 1;
    if (*s >= '0' && *s <= '9') value = (*s - '0');
    else return 0;
    s++;
    while (*s != 0)
    {
       if (*s >= '0' && *s <= '9')
       {
           value = value * 10 + (*s - '0');
           s++;
       }
       else return 0;
    }

    return value * sign;
}

int abs(int n)
{
    int j;

    if (n >= 0) j = n; else j = -n;

	return j;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
void exit(int status)
{
	printf("\nProgram exit with a status code %d\n", status);
    printf("\n-----------------------------------------------------------");
    printf("------------\nAquila execution finished.\n");
    printf("Press <reset> on the FPGA board to reboot the cpu ...\n\n");
    while (1);
}
#pragma GCC pop_options

static int rand_seed = 27182;

void srand(unsigned int seed)
{
    rand_seed = (long) seed;
}

int rand(void)
{
    return(((rand_seed = rand_seed * 214013L + 2531011L) >> 16) & 0x7fff);
}

