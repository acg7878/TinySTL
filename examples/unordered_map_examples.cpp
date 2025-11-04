#include <iostream>
#include <mystl/unordered_map.h>
#include <string>
int main() {
  mystl::unordered_map<int, std::string> um;
  um.insert({1, "apple"});
  um.insert({2, "banana"});
  um.insert({3, "cherry"});
  um.insert({4, "date"});
  um.insert({5, "elderberry"});
  auto x = um.find(5);
  if (x != um.end()) {
	  std::cout << x->second << std::endl;
  }
  return 0;
}