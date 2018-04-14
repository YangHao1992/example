#include <vector>
#include <string>
#include <iostream>

class A {
  public:
    A(int a) : val(a) {}
    ~A() {}
    int get() {
      return val;
    }
  private:
    int val;
};

void test(const std::vector<A> vec) {
  for (std::vector<A>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
    // A a = vec[i];
    std::cout << "val = " << it->get() << std::endl;
  }
}

int main() {
  std::vector<A> vec;
  A a(1);
  A a2(2);
  vec.push_back(a);
  vec.push_back(a2);
  test(vec);
}
