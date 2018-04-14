#include <unordered_map>
#include <unordered_set>
#include <iostream>
using namespace std;

class Format {
  public:
  virtual ~Format() {
    std::cout << "析构Foramt" << std::endl;
  }
};

class Test : public Format {
  public:
  explicit Test(int i) {
    var = i;
    std::cout << "构造Test, addr:" << i << std::endl;
  }
  ~Test() {
    std::cout << "析构Test, addr:" << var << std::endl;
  }
  public:
  int var;
};

std::unordered_set<Test *> formatSet;

int main() {
  for (int i = 0; i < 10; ++i) {
    Test *te = new Test(i);
    formatSet.insert(te);
  }
  while (formatSet.size() > 0) {
    Test *mt = *(formatSet.begin());
    delete mt;
  }
  return 0;
}
