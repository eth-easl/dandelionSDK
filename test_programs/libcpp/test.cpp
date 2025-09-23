#include <iostream>
#include <fstream>

using namespace std;

int main(){
  // cout << "Hello World" << endl;
  std::fstream fs;
  fs.open("/stdio/stdout", std::fstream::out);
  fs << " test print " << endl;
  fs.close();

  cout << "stdout print" << endl;
  cerr << "stderr print" << endl;
  return 0;
}

