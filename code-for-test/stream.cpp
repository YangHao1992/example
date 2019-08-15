#include <iostream>
#include <sstream>
using namespace std;
int main() {
  int n = 10;
  std::string ip = "172.168.0.1";
  std::string str;
  stringstream stream;
  stream << ip << "#" << n;
  stream >> str;
  std::cout << str << std::endl;
}
