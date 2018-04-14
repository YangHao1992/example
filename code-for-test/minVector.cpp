#include <iostream>
#include <vector>
#include <algorithm>

int main() {
  std::vector<uint64_t> vec {124345, 4545634, 46463524, 452534, 3234, 65744634, 32432};
  auto smallest = std::min_element(std::begin(vec), std::end(vec));
  std::cout << "the min value is:" << *smallest << std::endl;
}
