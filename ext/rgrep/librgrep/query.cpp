#include "query.hpp"
#include "record.hpp"
#include "utils.hpp"

#include <iostream>
#include <algorithm>
#include <set>

using namespace std;
using namespace RGrep;

namespace RGrep {
  struct RankData {
    size_t pos;
    int rank_value;

    RankData(size_t pos, int rank_value) : pos(pos), rank_value(rank_value) {
    }

    bool operator<(const RankData &oth) {
      if(rank_value != oth.rank_value) {
        return rank_value < oth.rank_value;
      } else {
        return pos < oth.pos;
      }
    }
  };
}

Query::Query(const char *q, bool ins) : query(q), insensitive(ins) {
  parse();
}

Query::Query(const string &q, bool ins) : query(q), insensitive(ins) {
  parse();
}

void Query::parse() {
  if(query.find(',') != string::npos) {
    type = BEST_MATCH;
  } else if(query.find('&') != string::npos || query.find('|') != string::npos) {
    type = BOOLEAN;
  } else {
    type = SINGLE;
  }
}

void Query::run(Record &record_parser, const function<bool (Record &, int)> &callback) {
  switch(type) {
    case SINGLE:
      search_single(record_parser, callback);
      break;
    case BEST_MATCH:
      search_multi(record_parser, callback);
      break;
    case BOOLEAN:
      search_boolean(record_parser, callback);
      break;
  }
}

void Query::search_single(Record &record_parser, const function<bool (Record &, int)> &callback) {
  BMSearch query_search(query);
  while(record_parser.search(query_search, insensitive)) {
    if(!callback(record_parser, 0)) {
      break;
    }
    record_parser.next_record();
  }
}

void Query::search_boolean(Record &record_parser, const function<bool (Record &, int)> &callback) {
  set<size_t> found_record;
  vector<vector<string>> compiled_pattern;
  vector<BMSearch> pat_searches;

  compile_boolean(compiled_pattern);

  for(auto term : compiled_pattern) {
    int len = term.size();

    for(auto pattern : term) {
      pat_searches.push_back(BMSearch(pattern));
    }

    while(record_parser.search_record(pat_searches[0], insensitive)) {
      bool all_match = true;
      for(int i = 1; i < len; ++i) {
        if(!record_parser.search_in_record(pat_searches[i], insensitive)) {
          all_match = false;
          break;
        }
      }

      if(all_match) {
        found_record.insert(record_parser.get_position());
      }

      record_parser.next_record();
    }
  }

  for(size_t pos : found_record) {
    record_parser.set_position(pos);
    if(!callback(record_parser, 0)) {
      break;
    }
  }
}

void Query::search_multi(Record &record_parser, const function<bool(Record &, int)> &callback) {
  vector<string> must_patterns;
  vector<string> not_patterns;
  vector<string> patterns;
  vector<BMSearch> must_searches, not_searches, pattern_searches;
  vector<RankData> found_record;
  set<size_t> explored_set;
  int must_len, pat_len, rank_value;
  size_t pos;
  compile_multi(must_patterns, not_patterns, patterns);

  for(auto pat : must_patterns) {
    must_searches.push_back(BMSearch(pat));
  }
  for(auto pat : not_patterns) {
    not_searches.push_back(BMSearch(pat));
  }
  for(auto pat : patterns) {
    pattern_searches.push_back(BMSearch(pat));
  }

  must_len = must_patterns.size();
  pat_len = patterns.size();
  if(must_len) { // If have must have pattern
    while(record_parser.search_record(must_searches[0], insensitive)) {
      bool skip_record = false;
      pos = record_parser.get_position();
      rank_value = 0;
      for(int i = 1; i < must_len; ++i) {
        if(!record_parser.search_in_record(must_searches[i], insensitive)) {
          skip_record = true;
          break;
        }
      }

      if(!skip_record) {
        for(auto matcher : not_searches) {
          if(record_parser.search_in_record(matcher, insensitive)) {
            skip_record = true;
            break;
          }
        }
      }

      if(!skip_record) {
        for(auto matcher : pattern_searches) {
          if(record_parser.search_in_record(matcher, insensitive)) {
            rank_value += 10;
          }
        }

        found_record.push_back(RankData(pos, rank_value));
        push_heap(found_record.begin(), found_record.end());
      }

      record_parser.next_record();
    }
  } else {
    for(int i = 0; i < pat_len; ++i) {
      while(record_parser.search_record(pattern_searches[i], insensitive)) {
        bool skip_record = false;
        rank_value = 10;
        pos = record_parser.get_position();
        if(explored_set.find(pos) != explored_set.end()) {
          skip_record = true;
        }

        if(!skip_record) {
          for(auto matcher : not_searches) {
            if(record_parser.search_in_record(matcher, insensitive)) {
              skip_record = true;
              break;
            }
          }
        }

        if(!skip_record) {
          for(int j = 0; j < pat_len; ++j) {
            if(j == i) {
              continue;
            }
            if(record_parser.search_in_record(pattern_searches[j], insensitive)) {
              rank_value += 10;
            }
          }

          found_record.push_back(RankData(pos, rank_value));
          push_heap(found_record.begin(), found_record.end());
        }

        explored_set.insert(pos);
        record_parser.next_record();
      }
      record_parser.rewind();
    }
  }

  while(!found_record.empty()) {
    pos = found_record[0].pos;
    rank_value = found_record[0].rank_value;
    record_parser.set_position(pos);

    if(!callback(record_parser, rank_value)) {
      break;
    }

    pop_heap(found_record.begin(), found_record.end());
    found_record.pop_back();
  }
}

void Query::compile_boolean(vector<vector<string>> &compiled_pattern) {
  vector<string> tokens;
  vector<string> term;
  string_split(tokens, query);

  for(auto token : tokens) {
    if(token != "&" && token != "|") {
      term.push_back(token);
    } else if(token == "|") {
      compiled_pattern.push_back(term);
      term.clear();
    }
  }
  if(!term.empty()) {
    compiled_pattern.push_back(term);
  }
}

void Query::compile_multi(
  vector<string> &must_patterns,
  vector<string> &not_patterns,
  vector<string> &patterns
) {
  vector<string> tokens;
  string_split(tokens, query, ", ");
  for(auto token : tokens) {
    if(token[0] == '+') {
      must_patterns.push_back(token.substr(1));
    } else if (token[0] == '-') {
      not_patterns.push_back(token.substr(1));
    } else {
      patterns.push_back(token);
    }
  }
}

