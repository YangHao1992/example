#include <atomic>
#include <iostream>
using namespace std;

int main() {
  std::atomic<bool> b;
  b = true;
  if (b) {
    std::cout << "true" << std::endl;
  }
  b = false;
  if (!b) {
    std::cout << "false" << std::endl;
  }
}
