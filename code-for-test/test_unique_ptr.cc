#include <memory>
#include <iostream>

class Test {
  public:
    int a;
    Test(int var) : a(var) {}
    ~Test() {}
};

void change(Test* te) {
  te->a = 10;
}

int main() {
  std::unique_ptr<Test> test(new Test(1));
  change(test.get());
  std::cout << "a = " << test->a << std::endl;
}
