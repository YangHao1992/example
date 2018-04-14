#include <iostream>
class Test {
  public:
    Test() {
    }
    ~Test() {
    }
    void run2() {
      std::lock_guard<std::mutex> lock(compactSnapshotLock);
      std::cout << __func__ << std::endl;
    }
    void run1() {
      std::lock_guard<std::mutex> lock(compactSnapshotLock);
      run2();
      std::cout << __func__ << std::endl;
    }
  private:
    std::mutex compactSnapshotLock;
};
int main() {
  Test *test = new Test();
  test->run1();
  delete test;
}
