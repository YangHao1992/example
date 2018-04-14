#include <iostream>
#include <thread>
#include <memory>
#include <unistd.h>
using namespace std;

class Thread {
  public:
    Thread() :isClearThreadInUse(false) {
      thr.reset(new std::thread());
    }
    ~Thread() {
      thr->join();
    }
    void start() {
      std::unique_lock<std::mutex> lock(clearMutex);
      isClearThreadInUse = true;
      lock.unlock();
      if (thr->joinable()) {
        thr->join();
      }
      thr.reset(new std::thread(&Thread::Print, this));
    }
    void Print() {
      std::cout << "Print." << std::endl;
      sleep(5);
      std::unique_lock<std::mutex> lock(clearMutex);
      isClearThreadInUse = false;
    }

    bool getSign() {
      std::unique_lock<std::mutex> lock(clearMutex);
      return isClearThreadInUse;
    }
  private:
    std::unique_ptr<std::thread> thr;
    std::mutex clearMutex;
    bool isClearThreadInUse;
};

int main() {
  std::unique_ptr<Thread> th(new Thread());
  int count = 0;
  while (count < 20) {
    if (!th->getSign()) {
      th->start();
    } else {
      std::cout << "the last clear task hasn't over, please wait the next timer." << std::endl;
    }
    sleep(5);
    count++;
  }
}
