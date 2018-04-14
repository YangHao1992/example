#include <iostream>

void print(int type) {
  switch(type) {
    case 0:
      std::cout << "0" << std::endl;
    case 1:
      std::cout << "1" << std::endl;

    std::cout << "12" << std::endl;
    break;

    default:
    break;
  }
}
int main() {
  print(1);
  std::cout << "------------" << std::endl;
  print(0);
}
