#ifndef RECORD_HPP_INCLUDE
#define RECORD_HPP_INCLUDE

#include <string>
#include <map>
#include <experimental/optional>

#include "bm_search.hpp"

namespace RGrep {
  class Record {
  public:
    Record(const char *begin_pattern);
    Record(const char *begin_pattern, const char *fname);
    ~Record();
    std::experimental::optional<size_t> search(const char *pattern, bool insensitive = false);
    std::experimental::optional<size_t> search(BMSearch &bm_search, bool insensitive = false);
    bool search_record(const char *pattern, bool insensitive = false);
    bool search_record(BMSearch &bm_search, bool insensitive = false);
    bool search_in_record(const char *pattern, bool insensitive = false);
    bool search_in_record(BMSearch &bm_search, bool insensitive = false);
    char *get_record();
    bool next_record();
    size_t get_position() const;
    void set_position(size_t position);
    std::experimental::optional<char *> get_attribute(const char *attr);
    void rewind();

  private:
    std::string filename;
    int fd;
    char *fptr, *begin_pat;
    size_t fsize, pat_len, pos;
    bool end_record;
    BMSearch begin_search, attr_start_search;
    std::map<std::string, BMSearch> attr_search;

    void init(const char *begin_pattern);
    int find_prev_record(size_t shift_pos);
    int find_next_record(size_t shift_pos);
    void map_file(const char *fname);
    void unmap_file();
  };
}

#endif /* end of include guard: RECORD_HPP_INCLUDE */

