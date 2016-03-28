#include "bm_search.hpp"

#include <iostream>
#include <cstring>
#include <cctype>

using namespace std;
using namespace RGrep;

BMSearch::BMSearch(const char *pat) : pattern(pat) {
  pattern_len = pattern.size();
  build_shift_table();
}

BMSearch::BMSearch(const string &pat) : pattern(pat) {
  pattern_len = pattern.size();
  build_shift_table();
}

int BMSearch::search(char *str, size_t len, size_t start_pos) const {
  char *suffix_orig = new char[pattern_len + 1];
  char *pos = str, *str_end = str + len;
  int match_pos = -1;

  if(len < start_pos) {
    return -1;
  }
  if(pattern_len > len) {
    return -1;
  }

  fill_suffix(str, len, suffix_orig); // Fill end char
  pos += start_pos; // Shift start pos

  while(pos < str_end) {
    register size_t idx = static_cast<size_t>(static_cast<unsigned char>(*pos));
    while(shift_table[idx]) {
      pos += shift_table[idx]; // Jump until match correct end char
      idx = static_cast<size_t>(static_cast<unsigned char>(*pos));
    }

    pos -= pattern_len - 1;

    if(strncmp(pos, pattern.c_str(), pattern_len) == 0) {
      match_pos = pos - str;
      break;
    }
    pos += fail_shift; // Not match, jump over
  }

  restore_suffix(str, len, suffix_orig);

  if(!~match_pos) { // match_pos == -1
    pos -= pattern_len + 1;
    pos = strstr(pos, pattern.c_str());
    if(pos) {
      match_pos = pos - str;
    }
  }

  delete [] suffix_orig;
  return match_pos;
}

int BMSearch::isearch(char *str, size_t len, size_t start_pos) const {
  char *suffix_orig = new char[pattern_len + 1];
  char *pos = str, *str_end = str + len;
  int match_pos = -1;

  if(len < start_pos) {
    return -1;
  }
  if(pattern_len > len) {
    return -1;
  }

  fill_suffix(str, len, suffix_orig); // Fill end char
  pos += start_pos; // Shift start pos

  while(pos < str_end) {
    register size_t idx = static_cast<size_t>(static_cast<unsigned char>(tolower(*pos)));
    while(ishift_table[idx]) {
      pos += shift_table[idx]; // Jump until match correct end char
      idx = static_cast<size_t>(static_cast<unsigned char>(tolower(*pos)));
    }

    pos -= pattern_len - 1;
    // cerr << (void *)pos << endl;

    if(strncasecmp(pos, pattern.c_str(), pattern_len) == 0) {
      match_pos = pos - str;
      break;
    }

    pos += fail_ishift; // Not match, jump over
  }

  restore_suffix(str, len, suffix_orig);

  if(!~match_pos) { // match_pos == -1
    pos -= pattern_len + 1;
    pos = strcasestr(pos, pattern.c_str());
    if(pos) {
      match_pos = pos - str;
    }
  }

  delete [] suffix_orig;
  return match_pos;
}

int BMSearch::rsearch(char *str, size_t len, size_t start_pos) const {
  char *prefix_orig = new char[pattern_len + 1];
  char *pos = str;
  int match_pos = -1;

  if(len < start_pos) {
    return -1;
  }
  if(pattern_len > len) {
    return -1;
  }
  if(pattern_len > start_pos) {
    return -1;
  }

  fill_prefix(str, prefix_orig); // Fill start char
  pos += start_pos; // Shift start pos

  while(pos > str) {
    register size_t idx = static_cast<size_t>(static_cast<unsigned char>(*pos));
    while(rshift_table[idx]) {
      pos -= rshift_table[idx]; // Jump until match correct end char
      idx = static_cast<size_t>(static_cast<unsigned char>(*pos));
    }

    if(strncmp(pos, pattern.c_str(), pattern_len) == 0) {
      match_pos = pos - str;
      break;
    }
    pos -= fail_rshift; // Not match, jump over
  }

  restore_prefix(str, prefix_orig);

  if(!~match_pos) { // match_pos == -1
    pos = str;
    pos = strstr(pos, pattern.c_str());
    if(pos) {
      match_pos = pos - str;
    }
  }

  delete [] prefix_orig;
  return match_pos;
}

void BMSearch::build_shift_table() {
  shift_table[0] = rshift_table[0] = ishift_table[0] = 0;

  for(int i = 0; i < 256; ++i) {
    ishift_table[i] = pattern_len;
  }

  for(int i = 1; i < 256; ++i) {
    string::size_type pos = pattern.rfind(char(i));
    int lower = tolower(i);

    if(pos == string::npos) {
      ishift_table[i] = shift_table[i] = pattern_len;
    } else {
      shift_table[i] = pattern_len - pos - 1;
      ishift_table[lower] = min(ishift_table[lower], shift_table[i]);
      if(shift_table[i] == 0) {
        pos = pattern.rfind(char(i), pattern_len - 2);
        fail_shift = (pos == string::npos ? 0 : pos) + pattern_len + 1;
      }
      if(ishift_table[lower] == 0) {
        pos = pattern.rfind(char(i), pattern_len - 2);
        fail_ishift = (pos == string::npos ? 0 : pos) + pattern_len + 1;
        pos = pattern.rfind(lower, pattern_len - 2);
        fail_ishift = min(fail_ishift, (pos == string::npos ? 0 : pos) + pattern_len + 1);
      }
    }

    pos = pattern.find(char(i));

    if(pos == string::npos) {
      rshift_table[i] = pattern_len;
    } else {
      rshift_table[i] = pos;
      if(rshift_table[i] == 0) {
        pos = pattern.find(char(i), 1);
        fail_rshift = (pos == string::npos ? 0 : pos) + pattern_len + 1;
      }
    }
  }

  // cout << endl << "Pattern: " << pattern << endl;
  // for(int i = 0; i < 256; ++i) {
  //   cout << char(i) << ": " << ishift_table[i] << endl;
  // }
}

void BMSearch::fill_suffix(char *str, size_t len, char *suffix_orig) const {
  char *ptr = nullptr;
  char ch;

  // Let ptr point to str[-pattern_len]
  ptr = str + len - pattern_len - 1;
  strncpy(suffix_orig, ptr, pattern_len + 1); // Backup str suffix
  ch = pattern[pattern_len - 1]; // Fetch last char in pattern
  memset(ptr, ch, pattern_len); // Fill suffix with char
}

void BMSearch::restore_suffix(char *str, size_t len, char *suffix_orig) const {
  char *ptr = nullptr;

  // Let ptr point to str[-pattern_len]
  ptr = str + len - pattern_len - 1;
  strcpy(ptr, suffix_orig); // Restore str suffix
}

void BMSearch::fill_prefix(char *str, char *prefix_orig) const {
  char *ptr = nullptr;
  char ch;

  // Let ptr point to str
  ptr = str;
  strncpy(prefix_orig, ptr, pattern_len + 1); // Backup str prefix
  ch = pattern[0]; // Fetch last char in pattern
  memset(ptr, ch, pattern_len); // Fill prefix with char
}

void BMSearch::restore_prefix(char *str, char *prefix_orig) const {
  char *ptr = nullptr;

  // Let ptr point to str
  ptr = str;
  strncpy(ptr, prefix_orig, pattern_len); // Restore str prefix
}

