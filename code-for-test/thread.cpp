#include <thread>
#include <iostream>
#include <utility>
#include <system_error>
 
class CThread {
public:
	CThread() : _bRun(false){}
  CThread(CThread& src): _bRun(src._bRun), _thread(move(src._thread)) {
    src._bRun = false;
  } 
	virtual ~CThread() {
		if(_thread.joinable())
			_thread.detach();
	}
 
	template <typename F>
	bool Start(F&& f) {
		if(_bRun) return false;
	  _bRun = true;
		_thread = std::thread(std::forward<F>(f));
		return _bRun;
	}
 
	bool IsRun() const {return _bRun;}
	void Join() {
		bool bRun = _bRun;
		_bRun = false;
		if (bRun)
			_thread.join();
	}
 
private:
	bool 		_bRun;
	std::thread	_thread;
};


void ThreadFunc1(void* pVoid) {
  std::cout << "-------------" << std::endl;
}

int main() {
  CThread		_thread;
  _thread.Start(ThreadFunc1);
}
