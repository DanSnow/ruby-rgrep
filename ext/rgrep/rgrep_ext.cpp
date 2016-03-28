#include <iostream>
#include <string>

#include <rice/Class.hpp>
#include <rice/Hash.hpp>

#include "rgrep.h"

using namespace std;
using namespace Rice;

Object rgrep_initialize(Object self, String query, String filename, Hash opts) {
  self.iv_set("@query", query);
  self.iv_set("@filename", filename);
  self.iv_set("@opts", opts);
  return Qnil;
}

Object hello_world(Object self) {
  puts("Hello world");
  return Qnil;
}

extern "C"
void Init_rgrep_ext() {
  Module rgrep_module = define_module("RGrep");
  Module rgrep_ext_module = define_module_under(rgrep_module, "Ext");
  Class rgrep_class = define_class_under(rgrep_ext_module, "RGrep")
    .define_method("initialize", &rgrep_initialize)
    .define_method("hello", &hello_world);
}
