#include <iostream>

int main() {
  while (true) {
    for (int i = 0; i < 10; ++i) {
      if (i == 1) break;
    }
    std::cout << __func__ << std::endl;
  }
}
