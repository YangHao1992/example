#include <iostream>

void test(std::string &str) {
  str = "1232";
}

int main() {
  std::string str;
  test(str);
  std::cout << "str:" << str << std::endl;
}
