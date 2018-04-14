#include <iostream>
static int count = 0;
class A {
  public:
    A() {
      count++;
      std::cout << "A count = " << count << std::endl;
    }
    ~A() {}
    int getCount() {
      return count;
    }
};

class B : public A {
  public:
    B() : A() {
      std::cout<<"B Constructed\n";
    }
};

int main() {
  A a;
  B b;
  std::cout << "count:" << b.getCount() << std::endl;
}
