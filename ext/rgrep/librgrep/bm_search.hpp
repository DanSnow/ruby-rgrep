#ifndef BM_SEARCH_HPP_INCLUDE
#define BM_SEARCH_HPP_INCLUDE

#include <cstddef>
#include <string>

namespace RGrep {
  class BMSearch {
   public:
     BMSearch(const char *pat);
     BMSearch(const std::string &pat);

     int search(char *str, size_t len, size_t start_pos = 0) const;
     int isearch(char *str, size_t len, size_t start_pos = 0) const;
     int rsearch(char *str, size_t len, size_t start_pos = 0) const;
   private:
     void build_shift_table();
     void fill_prefix(char *str, char *orig) const;
     void fill_suffix(char *str, size_t len, char *orig) const;
     void restore_prefix(char *str, char *orig) const;
     void restore_suffix(char *str, size_t len, char *orig) const;

     std::string pattern;
     size_t pattern_len, fail_shift, fail_rshift, fail_ishift;
     unsigned int shift_table[256];
     unsigned int ishift_table[256];
     unsigned int rshift_table[256];
  };
}

#endif /* end of include guard: BM_SEARCH_HPP_INCLUDE */

