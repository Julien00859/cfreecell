#include "isort.h"

/**
 * This macro is from glibc which is distributed under the terms of
 * the GNU Lesser General Public License.
 */
#define SWAP(a, b, size)			\
  do								\
	{							   \
	  size_t __size = (size);	   \
	  char *__a = (a), *__b = (b);  \
	  do							\
		{						   \
		  char __tmp = *__a;		\
		  *__a++ = *__b;			\
		  *__b++ = __tmp;		   \
		} while (--__size > 0);	 \
	} while (0)

/**
 * Adaptation of the qsort_r sort function as defined in qsort(3)
 * using the insertion-sort algorithm which is suited to sort small
 * arrays.
 */
void isort_r(void *base, size_t nmemb, size_t size,
			 int (*compar)(const void *, const void *, const void *),
			 void *arg) {
	void *pi, *pj;
	for(pi=base + size; pi < base + nmemb * size; pi += size)
		for(pj = pi; pj > base && compar(pj - size, pj, arg) > 0; pj -= size)
			SWAP(pj, pj - size, size);
}
