#include "record.hpp"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;
using namespace std::experimental;
using namespace RGrep;

Record::Record(const char *begin_pattern) : begin_search(begin_pattern), attr_start_search("\n@") {
  init(begin_pattern);
}

Record::Record(const char *begin_pattern, const char *fname) : begin_search(begin_pattern), attr_start_search("\n@") {
  init(begin_pattern);
  filename = fname;
  map_file(fname);
  cerr << "File load" << endl;
}

void Record::init(const char *begin_pattern) {
  begin_pat = strdup(begin_pattern);
  pat_len = strlen(begin_pat);
  pos = 0;
  end_record = false;
  fd = -1;
}

void Record::rewind() {
  end_record = false;
  pos = 0;
}

bool Record::search_record(const char *pattern, bool insensitive) {
  return bool(search(pattern, insensitive));
}

bool Record::search_record(BMSearch &bm_search, bool insensitive) {
  return bool(search(bm_search, insensitive));
}

bool Record::search_in_record(const char *pattern, bool insensitive) {
  BMSearch pat_search(pattern);
  return search_in_record(pat_search, insensitive);
}

bool Record::search_in_record(BMSearch &pat_search, bool insensitive) {
  size_t orig_pos = pos; // Save original pos
  bool result = false;

  // Find pattern
  if(search_record(pat_search, insensitive)) {
    if(orig_pos == pos) { // If at same record
      result = true;
    }
  }

  pos = orig_pos; // Restore pos

  return result;
}

size_t Record::get_position() const {
  return pos;
}

void Record::set_position(size_t position) {
  pos = position;

  if(end_record && pos != fsize) { // Clear end flag
    end_record = false;
  }
}

optional<size_t> Record::search(const char *pattern, bool insensitive) {
  BMSearch pat_search(pattern);
  return search(pat_search, insensitive);
}

optional<size_t> Record::search(BMSearch &pat_search, bool insensitive) {
  optional<size_t> result;
  int pat_pos, prev_pos;

  if(insensitive) {
    pat_pos = pat_search.isearch(fptr, fsize, pos);
  } else {
    pat_pos = pat_search.search(fptr, fsize, pos);
  }

  if(!~pat_pos) {
    return result;
  }
  if(end_record) {
    return result;
  }

  result = pat_pos;
  prev_pos = find_prev_record(pat_pos);

  if(prev_pos == -1) {
    pos = 0;
  } else {
    pos = prev_pos;
  }

  return result;
}

char *Record::get_record() {
  size_t prev_pos = pos;
  char *result = nullptr;

  if(!next_record()) {
    return nullptr;
  }

  result = strndup(fptr + prev_pos, pos - prev_pos);
  return result;
}

bool Record::next_record() {
  int new_pos = -1;
  if(end_record) {
    return false;
  }

  if(pos == fsize) {
    end_record = true;
    return false;
  }

  new_pos = find_next_record(pos);

  if(!~new_pos) {
    end_record = true;
    new_pos = fsize - 1;
  }

  pos = new_pos;
  return true;
}

optional<char *> Record::get_attribute(const char *attr) {
  char buffer[255];
  int attr_pos, attr_end_pos;
  size_t len;
  optional<char *> attr_content;
  len = sprintf(buffer, "@%s:", attr);
  auto it = attr_search.find(buffer);
  if(it == attr_search.end()) {
    auto res = attr_search.emplace(buffer, BMSearch(buffer));
    if(res.second) {
      it = res.first;
    }
  }
  attr_pos = it->second.search(fptr, fsize, pos);
  if(~attr_pos) { // attr_pos != -1
    char *content_pos;
    size_t content_len;
    attr_end_pos = attr_start_search.search(fptr, fsize, attr_pos + len);

    content_pos = fptr + attr_pos + len;
    content_len = attr_end_pos - attr_pos - len;

    attr_content = strndup(content_pos, content_len);
  }
  return attr_content;
}

int Record::find_prev_record(size_t shift_pos) {
  int prev_pos = begin_search.rsearch(fptr, fsize, shift_pos);
  if(prev_pos - shift_pos) {
    return prev_pos;
  }

  shift_pos -= pat_len;
  return begin_search.rsearch(fptr, fsize, shift_pos);
}

int Record::find_next_record(size_t shift_pos) {
  int next_pos = begin_search.search(fptr, fsize, shift_pos);
  if(next_pos - shift_pos) {
    return next_pos;
  }

  shift_pos += pat_len;
  return begin_search.search(fptr, fsize, shift_pos);
}

Record::~Record() {
  if(fd != -1) {
    unmap_file();
  }
  free(begin_pat);
}

void Record::map_file(const char *fname) {
  struct stat st;
  fd = open(fname, O_RDONLY);
  if(fd == -1) {
    perror("open");
    return;
  }
  if(fstat(fd, &st) == -1) {
    perror("fstat");
    return;
  }
  if(!S_ISREG(st.st_mode)) {
    cerr << fname << " not a file" << endl;
  }
  fsize = st.st_size + 1;
  fptr = (char *)mmap(0,
      fsize,
      PROT_READ | PROT_WRITE,
      MAP_PRIVATE,
      fd,
      0
  );
  if(fptr == MAP_FAILED) {
    perror("mmap");
    return;
  }
  if(close(fd) == -1) {
    perror("close");
    return;
  }

  fptr[fsize] = '\0';
}

void Record::unmap_file() {
  if(munmap(fptr, fsize) == -1) {
    perror("munmap");
    return;
  }
}
