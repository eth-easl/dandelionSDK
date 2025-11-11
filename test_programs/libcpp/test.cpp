#include <iostream>
#include <fstream>
#include <dirent.h>

using namespace std;

int main(){
  std::fstream fs;
  fs.open("/stdio/stdout", std::fstream::out);
  fs << "test print " << endl;
  fs.close();

  std::fstream in1;   
  string in_file_1 = "/in_a/input_1.txt";
  in1.open(in_file_1, std::fstream::in);
  if (!in1) {
    cerr << "Failed to open " << in_file_1 << endl;
    return -1;
  }
  cout << in1.rdbuf() << endl;
  in1.close();
  std::fstream in2;   
  string in_file_2 = "/in_b/input_2.txt";
  in2.open(in_file_2, std::fstream::in);
  if (!in2) {
    cerr << "Failed to open " << in_file_2 << endl;
    return -1;
  }
  cerr << in2.rdbuf() << endl;
  in2.close();

  cout << "stdout print" << endl;
  cerr << "stderr print" << endl;
  return 0;
}

