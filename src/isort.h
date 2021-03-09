#ifndef FREECELL_ISORT_H
#define FREECELL_ISORT_H

void isort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, const void *),
             void *arg);

#endif
