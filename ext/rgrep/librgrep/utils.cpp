#include "utils.hpp"

using namespace std;

void string_split(vector<string> &result, string &str, const char *delit) {
  size_t prev_pos = 0, pos = 0;
  string token;
  while(true) {
    pos = str.find_first_of(delit, prev_pos);
    token = str.substr(prev_pos, pos - prev_pos);
    if(!token.empty()) {
      result.push_back(token);
    }
    prev_pos = pos + 1;

    if(pos == string::npos) {
      break;
    }
  }
}
