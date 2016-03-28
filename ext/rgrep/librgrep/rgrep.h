#ifndef RGREP_H_INCLUDE
#define RGREP_H_INCLUDE

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
  struct _BMSearch;
  typedef struct _BMSearch BMSearch;

  BMSearch *bm_new(const char *pattern);
  int bm_search(const BMSearch *bm, char *str, size_t len);
  void bm_destroy(BMSearch *bm);

#ifdef __cplusplus
}

#include "bm_search.hpp"
#include "record.hpp"
#include "query.hpp"
#include "utils.hpp"
#endif

#endif /* end of include guard: RGREP_H_INCLUDE */

