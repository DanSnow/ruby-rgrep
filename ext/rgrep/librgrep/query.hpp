#ifndef QUERY_HPP_INCLUDE
#define QUERY_HPP_INCLUDE

#include <string>
#include <vector>
#include <functional>

namespace RGrep {
  class Record;

  class Query {
  public:
    enum Type {
      BEST_MATCH,
      BOOLEAN,
      SINGLE
    };

    Query(const char *q, bool);
    Query(const std::string &q, bool);
    void run(Record &record, const std::function<bool (Record &, int)> &);

  private:
    std::string query;
    Type type;
    bool insensitive;

    void parse(void);
    void search_single(Record &record, const std::function<bool (Record &, int)> &);
    void search_boolean(Record &record, const std::function<bool (Record &, int)> &);
    void search_multi(Record &record, const std::function<bool (Record &, int)> &);
    void compile_boolean(std::vector<std::vector<std::string>> &compiled_pattern);
    void compile_multi(
        std::vector<std::string> &must_patterns,
        std::vector<std::string> &not_patterns,
        std::vector<std::string> &patterns
    );
  };
}

#endif /* end of include guard: QUERY_HPP_INCLUDE */

