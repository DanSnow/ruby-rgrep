#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <rice/Class.hpp>
#include <rice/Hash.hpp>
#include <rice/Symbol.hpp>

#include "rgrep.h"

using namespace std;
using namespace Rice;
using namespace RGrep;

Object rgrep_initialize(Object self, String query, String filename, Hash opts) {
  self.iv_set("@query", query);
  self.iv_set("@filename", filename);
  self.iv_set("@opts", opts);
  Module(rb_mKernel).call("puts", self.iv_get("@opts"));
  Hash ins_opts = self.iv_get("@opts");
  Module(rb_mKernel).call("puts", ins_opts.get<Array, Symbol>(Symbol("cols")));
  return Qnil;
}

Object rgrep_run(Object self) {
  string query_str = from_ruby<string>(self.iv_get("@query"));
  Hash opts = self.iv_get("@opts");
  string filename = from_ruby<string>(self.iv_get("@filename"));
  string begin_pat = from_ruby<string>(opts[Symbol("begin_pat")]);
  bool insensitive = from_ruby<bool>(opts[Symbol("insensitive")]);
  Array columns_arr = opts.get<Array, Symbol>(Symbol("cols"));
  vector<string> columns;
  int column_len = columns_arr.size();
  Record record(begin_pat.c_str(), filename.c_str());
  Query query(query_str, insensitive);

  for(auto it = columns_arr.begin(); it != columns_arr.end(); ++it) {
    columns.push_back(from_ruby<string>(*it));
  }

  query.run(record, [&columns, &column_len](Record &record_parser, int rank_value) {
    Hash data;
    data[String("rank")] = rank_value;
    for(int i = 0; i < column_len; ++i) {
      char *attr = record_parser.get_attribute(columns[i].c_str()).value();
      String value = attr;
      Object(value).call("force_encoding", "UTF-8");
      data[to_ruby(columns[i])] = value;
      free(attr);
    }
    protect(rb_yield, data);
    return true;
  });
  return Qnil;
}

extern "C"
void Init_rgrep_ext() {
  Module rgrep_module = define_module("RGrep");
  Module rgrep_ext_module = define_module_under(rgrep_module, "Ext");
  Class rgrep_class = define_class_under(rgrep_ext_module, "RGrep")
    .define_method("initialize", &rgrep_initialize)
    .define_method("run", &rgrep_run);
}
