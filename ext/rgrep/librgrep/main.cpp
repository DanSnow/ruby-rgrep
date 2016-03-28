#include "rgrep.h"
#include "bm_search.hpp"

struct _BMSearch {
  _BMSearch(const char *pattern) : bm_search(pattern) {
  }
  RGrep::BMSearch bm_search;
};

extern "C" {
  BMSearch *bm_new(const char *pattern) {
    return new BMSearch(pattern);
  }

  int bm_search(const BMSearch *bm, char *str, size_t len) {
    return bm->bm_search.search(str, len);
  }

  void bm_destroy(BMSearch *bm) {
    free(bm);
  }
}
