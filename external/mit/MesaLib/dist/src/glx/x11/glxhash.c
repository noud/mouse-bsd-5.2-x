/* glxhash.c -- Small hash table support for integer -> integer mapping
 * Taken from libdrm.
 *
 * Created: Sun Apr 18 09:35:45 1999 by faith@precisioninsight.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors: Rickard E. (Rik) Faith <faith@valinux.com>
 *
 * DESCRIPTION
 *
 * This file contains a straightforward implementation of a fixed-sized
 * hash table using self-organizing linked lists [Knuth73, pp. 398-399] for
 * collision resolution.  There are two potentially interesting things
 * about this implementation:
 *
 * 1) The table is power-of-two sized.  Prime sized tables are more
 * traditional, but do not have a significant advantage over power-of-two
 * sized table, especially when double hashing is not used for collision
 * resolution.
 *
 * 2) The hash computation uses a table of random integers [Hanson97,
 * pp. 39-41].
 *
 * FUTURE ENHANCEMENTS
 *
 * With a table size of 512, the current implementation is sufficient for a
 * few hundred keys.  Since this is well above the expected size of the
 * tables for which this implementation was designed, the implementation of
 * dynamic hash tables was postponed until the need arises.  A common (and
 * naive) approach to dynamic hash table implementation simply creates a
 * new hash table when necessary, rehashes all the data into the new table,
 * and destroys the old table.  The approach in [Larson88] is superior in
 * two ways: 1) only a portion of the table is expanded when needed,
 * distributing the expansion cost over several insertions, and 2) portions
 * of the table can be locked, enabling a scalable thread-safe
 * implementation.
 *
 * REFERENCES
 *
 * [Hanson97] David R. Hanson.  C Interfaces and Implementations:
 * Techniques for Creating Reusable Software.  Reading, Massachusetts:
 * Addison-Wesley, 1997.
 *
 * [Knuth73] Donald E. Knuth. The Art of Computer Programming.  Volume 3:
 * Sorting and Searching.  Reading, Massachusetts: Addison-Wesley, 1973.
 *
 * [Larson88] Per-Ake Larson. "Dynamic Hash Tables".  CACM 31(4), April
 * 1988, pp. 446-457.
 *
 */

#include "glxhash.h"
#include <X11/Xfuncproto.h>

#define HASH_MAIN 0

#include <stdio.h>
#include <stdlib.h>

#define HASH_MAGIC 0xdeadbeef
#define HASH_DEBUG 0
#define HASH_SIZE  512          /* Good for about 100 entries */
                                /* If you change this value, you probably
                                   have to change the HashHash hashing
                                   function! */

#define HASH_ALLOC malloc
#define HASH_FREE  free

typedef struct __glxHashBucket
{
   unsigned long key;
   void *value;
   struct __glxHashBucket *next;
} __glxHashBucket, *__glxHashBucketPtr;

typedef struct __glxHashTable *__glxHashTablePtr;
struct __glxHashTable
{
   unsigned long magic;
   unsigned long hits;          /* At top of linked list */
   unsigned long partials;      /* Not at top of linked list */
   unsigned long misses;        /* Not in table */
   __glxHashBucketPtr buckets[HASH_SIZE];
   int p0;
   __glxHashBucketPtr p1;
};

static unsigned long
HashHash(unsigned long key)
{
 static unsigned long scatter[256]
  = { 0x1ea510b7, 0x616d1a49, 0x1656bae7, 0x68e2e2ff, 0x041b063c,
      0x523c0b0b, 0x2a0c0f95, 0x711f13b1, 0x2288f3c4, 0x4c8d1ea0,
      0x27f331e0, 0x39a90b6b, 0x674fe63b, 0x2f685346, 0x522edc8b,
      0x7a8e2d06, 0x15409676, 0x5c58e2df, 0x4bd025f2, 0x766b6e72,
      0x4e150ca9, 0x58510977, 0x3706b831, 0x13540a09, 0x754cb5e5,
      0x12c17a8f, 0x152437c5, 0x51e69d62, 0x68dff5a0, 0x7f9e9eee,
      0x171fdb15, 0x07850657, 0x610bb937, 0x2d7695fc, 0x7067e957,
      0x6526bf73, 0x7fb2a108, 0x1a73f8ec, 0x5645d325, 0x223b94cc,
      0x6701178d, 0x7e390505, 0x5be4a037, 0x4e50fdc8, 0x2da1584c,
      0x2e137cc2, 0x48df2ace, 0x42e1eec2, 0x0a6c5fa1, 0x14af50c0,
      0x394d5d34, 0x58816c4a, 0x6d005a37, 0x70541565, 0x6bd57654,
      0x624d101c, 0x03158ff4, 0x00f9ae19, 0x3433ad7f, 0x6bf58594,
      0x00984d07, 0x4b538894, 0x737a8bec, 0x61a4063e, 0x78ca1e90,
      0x63e27543, 0x46cac5b2, 0x787cbf98, 0x7e566e2f, 0x1d1098d7,
      0x1ab85464, 0x655785bc, 0x1b499ddc, 0x769cf49b, 0x33a88384,
      0x48eaf628, 0x24b0715e, 0x7c87ae52, 0x0bcce4ea, 0x2f1cd0ff,
      0x1136ff12, 0x451a421e, 0x079e3d4a, 0x7e375949, 0x356e5783,
      0x7373b39e, 0x60846966, 0x3883e777, 0x746d61b7, 0x14b816e5,
      0x24796d0c, 0x7505aebf, 0x600b9f79, 0x17f3f8f8, 0x56a9b4fd,
      0x58d5be09, 0x7bd66e3b, 0x1d747aaf, 0x51527da2, 0x7a2cdc6a,
      0x3a851386, 0x6c0ad206, 0x5f846227, 0x55ceb163, 0x62a7c6a2,
      0x132ce5ab, 0x1eb9a78b, 0x07583800, 0x0fb493fe, 0x2a868c76,
      0x367508ff, 0x20eb9310, 0x6fa0ce94, 0x3e134649, 0x1f22ec5a,
      0x250f2618, 0x3186f9e7, 0x7fa755c0, 0x5d930d8f, 0x25f45b9f,
      0x145f6ca5, 0x020c7a9b, 0x1afa0a5e, 0x746b0c1e, 0x1a007393,
      0x71a3bf5b, 0x4d40ca27, 0x15d6e1ce, 0x0f183a0b, 0x1e9347c9,
      0x1003be39, 0x499d4d91, 0x0a9e19d0, 0x6f882060, 0x1f6bfef4,
      0x6d45e072, 0x02b5060b, 0x3e25a680, 0x749e1872, 0x12699a09,
      0x68ac32f6, 0x2b132171, 0x33552d1a, 0x584d018a, 0x692667bb,
      0x52781974, 0x7d5c27a2, 0x1aad61a2, 0x521f6f34, 0x5aef3532,
      0x40a1bd41, 0x667edbd9, 0x5cfbafcd, 0x5b9bc79f, 0x5ae9e7f7,
      0x76fc2361, 0x4d3f86fb, 0x282ab21e, 0x0cd3052f, 0x5c57c106,
      0x46bdf9e8, 0x1cd6c368, 0x25f50e97, 0x515c13b8, 0x0c5ee3c8,
      0x45610d8c, 0x3ea1f42a, 0x0f13e9d4, 0x0386b40c, 0x33400c9c,
      0x217d83dd, 0x6c32e702, 0x5e532e0d, 0x54d2b0f7, 0x447fe88c,
      0x477995c8, 0x274aca6b, 0x41dc102f, 0x6226f76b, 0x796a399f,
      0x1ccb4561, 0x22c8b4ac, 0x5fe91578, 0x79c6f52e, 0x7e647c4c,
      0x3ad2fd6f, 0x70c3188f, 0x4ba40347, 0x62fdaf8e, 0x7d961dbf,
      0x27fbc44d, 0x29bba976, 0x1a6ce127, 0x4df0d2e4, 0x7b17bd2e,
      0x26cbc4f0, 0x1351e070, 0x39b9b158, 0x35dfaec4, 0x16d8947c,
      0x6cf9bdf4, 0x575d32a1, 0x030b7b7e, 0x4b4cec01, 0x2c2fe399,
      0x478b640b, 0x12c681ca, 0x537aae04, 0x0967743a, 0x74ed7935,
      0x4ce4e7a4, 0x2632b99b, 0x17b62de1, 0x2ccdfd1c, 0x1ff9aec9,
      0x161aaa2d, 0x67a0fa8c, 0x10bcc759, 0x61bead74, 0x4a9eaa1a,
      0x0e52e518, 0x09ba71c1, 0x745a5390, 0x28bfc63f, 0x57ab44a6,
      0x6f7210be, 0x4f8b8b2f, 0x6afd2516, 0x292bc216, 0x056b39f3,
      0x01d5b993, 0x1625800a, 0x5cc86c95, 0x04e13511, 0x61726c0b,
      0x08f8502e, 0x4c6c991c, 0x7438edd5, 0x5c72fe32, 0x55d40d56,
      0x6926670a, 0x2957e5d6, 0x7c06c6f1, 0x00dc94ec, 0x5625e2f3,
      0x1c0075bb, 0x16f73f19, 0x3dc6dd7f, 0x2cbd3d14, 0x78b5ec8e,
      0x08658799, 0x3b10222c, 0x02705e4f, 0x7cbfdb29, 0x63cfe86b,
      0x5a1ba2f5 };
   unsigned long hash = 0;
   unsigned long tmp = key;
   int i;

   while (tmp) {
      hash = (hash << 1) + scatter[tmp & 0xff];
      tmp >>= 8;
   }

   hash %= HASH_SIZE;
#if HASH_DEBUG
   printf("Hash(%d) = %d\n", key, hash);
#endif
   return hash;
}

_X_HIDDEN __glxHashTable *
__glxHashCreate(void)
{
   __glxHashTablePtr table;
   int i;

   table = HASH_ALLOC(sizeof(*table));
   if (!table)
      return NULL;
   table->magic = HASH_MAGIC;
   table->hits = 0;
   table->partials = 0;
   table->misses = 0;

   for (i = 0; i < HASH_SIZE; i++)
      table->buckets[i] = NULL;
   return table;
}

_X_HIDDEN int
__glxHashDestroy(__glxHashTable * t)
{
   __glxHashTablePtr table = (__glxHashTablePtr) t;
   __glxHashBucketPtr bucket;
   __glxHashBucketPtr next;
   int i;

   if (table->magic != HASH_MAGIC)
      return -1;                /* Bad magic */

   for (i = 0; i < HASH_SIZE; i++) {
      for (bucket = table->buckets[i]; bucket;) {
         next = bucket->next;
         HASH_FREE(bucket);
         bucket = next;
      }
   }
   HASH_FREE(table);
   return 0;
}

/* Find the bucket and organize the list so that this bucket is at the
   top. */

static __glxHashBucketPtr
HashFind(__glxHashTablePtr table, unsigned long key, unsigned long *h)
{
   unsigned long hash = HashHash(key);
   __glxHashBucketPtr prev = NULL;
   __glxHashBucketPtr bucket;

   if (h)
      *h = hash;

   for (bucket = table->buckets[hash]; bucket; bucket = bucket->next) {
      if (bucket->key == key) {
         if (prev) {
            /* Organize */
            prev->next = bucket->next;
            bucket->next = table->buckets[hash];
            table->buckets[hash] = bucket;
            ++table->partials;
         }
         else {
            ++table->hits;
         }
         return bucket;
      }
      prev = bucket;
   }
   ++table->misses;
   return NULL;
}

_X_HIDDEN int
__glxHashLookup(__glxHashTable * t, unsigned long key, void **value)
{
   __glxHashTablePtr table = (__glxHashTablePtr) t;
   __glxHashBucketPtr bucket;

   if (!table || table->magic != HASH_MAGIC)
      return -1;                /* Bad magic */

   bucket = HashFind(table, key, NULL);
   if (!bucket)
      return 1;                 /* Not found */
   *value = bucket->value;
   return 0;                    /* Found */
}

_X_HIDDEN int
__glxHashInsert(__glxHashTable * t, unsigned long key, void *value)
{
   __glxHashTablePtr table = (__glxHashTablePtr) t;
   __glxHashBucketPtr bucket;
   unsigned long hash;

   if (table->magic != HASH_MAGIC)
      return -1;                /* Bad magic */

   if (HashFind(table, key, &hash))
      return 1;                 /* Already in table */

   bucket = HASH_ALLOC(sizeof(*bucket));
   if (!bucket)
      return -1;                /* Error */
   bucket->key = key;
   bucket->value = value;
   bucket->next = table->buckets[hash];
   table->buckets[hash] = bucket;
#if HASH_DEBUG
   printf("Inserted %d at %d/%p\n", key, hash, bucket);
#endif
   return 0;                    /* Added to table */
}

_X_HIDDEN int
__glxHashDelete(__glxHashTable * t, unsigned long key)
{
   __glxHashTablePtr table = (__glxHashTablePtr) t;
   unsigned long hash;
   __glxHashBucketPtr bucket;

   if (table->magic != HASH_MAGIC)
      return -1;                /* Bad magic */

   bucket = HashFind(table, key, &hash);

   if (!bucket)
      return 1;                 /* Not found */

   table->buckets[hash] = bucket->next;
   HASH_FREE(bucket);
   return 0;
}

_X_HIDDEN int
__glxHashNext(__glxHashTable * t, unsigned long *key, void **value)
{
   __glxHashTablePtr table = (__glxHashTablePtr) t;

   while (table->p0 < HASH_SIZE) {
      if (table->p1) {
         *key = table->p1->key;
         *value = table->p1->value;
         table->p1 = table->p1->next;
         return 1;
      }
      table->p1 = table->buckets[table->p0];
      ++table->p0;
   }
   return 0;
}

_X_HIDDEN int
__glxHashFirst(__glxHashTable * t, unsigned long *key, void **value)
{
   __glxHashTablePtr table = (__glxHashTablePtr) t;

   if (table->magic != HASH_MAGIC)
      return -1;                /* Bad magic */

   table->p0 = 0;
   table->p1 = table->buckets[0];
   return __glxHashNext(table, key, value);
}

#if HASH_MAIN
#define DIST_LIMIT 10
static int dist[DIST_LIMIT];

static void
clear_dist(void)
{
   int i;

   for (i = 0; i < DIST_LIMIT; i++)
      dist[i] = 0;
}

static int
count_entries(__glxHashBucketPtr bucket)
{
   int count = 0;

   for (; bucket; bucket = bucket->next)
      ++count;
   return count;
}

static void
update_dist(int count)
{
   if (count >= DIST_LIMIT)
      ++dist[DIST_LIMIT - 1];
   else
      ++dist[count];
}

static void
compute_dist(__glxHashTablePtr table)
{
   int i;
   __glxHashBucketPtr bucket;

   printf("Hits = %ld, partials = %ld, misses = %ld\n",
          table->hits, table->partials, table->misses);
   clear_dist();
   for (i = 0; i < HASH_SIZE; i++) {
      bucket = table->buckets[i];
      update_dist(count_entries(bucket));
   }
   for (i = 0; i < DIST_LIMIT; i++) {
      if (i != DIST_LIMIT - 1)
         printf("%5d %10d\n", i, dist[i]);
      else
         printf("other %10d\n", dist[i]);
   }
}

static void
check_table(__glxHashTablePtr table, unsigned long key, unsigned long value)
{
   unsigned long retval = 0;
   int retcode = __glxHashLookup(table, key, &retval);

   switch (retcode) {
   case -1:
      printf("Bad magic = 0x%08lx:"
             " key = %lu, expected = %lu, returned = %lu\n",
             table->magic, key, value, retval);
      break;
   case 1:
      printf("Not found: key = %lu, expected = %lu returned = %lu\n",
             key, value, retval);
      break;
   case 0:
      if (value != retval)
         printf("Bad value: key = %lu, expected = %lu, returned = %lu\n",
                key, value, retval);
      break;
   default:
      printf("Bad retcode = %d: key = %lu, expected = %lu, returned = %lu\n",
             retcode, key, value, retval);
      break;
   }
}

int
main(void)
{
   __glxHashTablePtr table;
   int i;

   printf("\n***** 256 consecutive integers ****\n");
   table = __glxHashCreate();
   for (i = 0; i < 256; i++)
      __glxHashInsert(table, i, i);
   for (i = 0; i < 256; i++)
      check_table(table, i, i);
   for (i = 256; i >= 0; i--)
      check_table(table, i, i);
   compute_dist(table);
   __glxHashDestroy(table);

   printf("\n***** 1024 consecutive integers ****\n");
   table = __glxHashCreate();
   for (i = 0; i < 1024; i++)
      __glxHashInsert(table, i, i);
   for (i = 0; i < 1024; i++)
      check_table(table, i, i);
   for (i = 1024; i >= 0; i--)
      check_table(table, i, i);
   compute_dist(table);
   __glxHashDestroy(table);

   printf("\n***** 1024 consecutive page addresses (4k pages) ****\n");
   table = __glxHashCreate();
   for (i = 0; i < 1024; i++)
      __glxHashInsert(table, i * 4096, i);
   for (i = 0; i < 1024; i++)
      check_table(table, i * 4096, i);
   for (i = 1024; i >= 0; i--)
      check_table(table, i * 4096, i);
   compute_dist(table);
   __glxHashDestroy(table);

   printf("\n***** 1024 random integers ****\n");
   table = __glxHashCreate();
   srandom(0xbeefbeef);
   for (i = 0; i < 1024; i++)
      __glxHashInsert(table, random(), i);
   srandom(0xbeefbeef);
   for (i = 0; i < 1024; i++)
      check_table(table, random(), i);
   srandom(0xbeefbeef);
   for (i = 0; i < 1024; i++)
      check_table(table, random(), i);
   compute_dist(table);
   __glxHashDestroy(table);

   printf("\n***** 5000 random integers ****\n");
   table = __glxHashCreate();
   srandom(0xbeefbeef);
   for (i = 0; i < 5000; i++)
      __glxHashInsert(table, random(), i);
   srandom(0xbeefbeef);
   for (i = 0; i < 5000; i++)
      check_table(table, random(), i);
   srandom(0xbeefbeef);
   for (i = 0; i < 5000; i++)
      check_table(table, random(), i);
   compute_dist(table);
   __glxHashDestroy(table);

   return 0;
}
#endif
