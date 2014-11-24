#include "../base64.h"
#include <iostream>
#include <fstream>
#include <sys/time.h>

using namespace std;
using namespace tools::base64;

int main()
{
  struct timespec  ts;
  //jpeg -> base64
  ifstream infile ("org.jpg",ofstream::binary);
  // get size of file
  infile.seekg (0,infile.end);
  long size = infile.tellg();
  infile.seekg (0);
  // allocate memory for file content
  char* buffer = new char[size];
  // read content of infile
  infile.read (buffer,size);
  infile.close();

  int result_size = ((size % 3)? size/3+1: size/3) * 4;
  char* result_buf = new char[result_size+2000];
  clock_gettime(CLOCK_REALTIME, &ts);
  printf ("------------------------------------------------ %ld %ld\n", ts.tv_sec, ts.tv_nsec);

  base64e(buffer, result_buf, size);
  clock_gettime(CLOCK_REALTIME, &ts);
  printf ("------------------------------------------------ %ld %ld\n", ts.tv_sec, ts.tv_nsec);

  base64e2_http(buffer, size, result_buf);
  
  clock_gettime(CLOCK_REALTIME, &ts);
  printf ("------------------------------------------------ %ld %ld\n", ts.tv_sec, ts.tv_nsec);
  std::ofstream oOut("resultbyEncode.txt");
  oOut << result_buf;
  oOut.close();
  delete result_buf;


  //base64 -> jpeg
  string encoded;
  infile.open("resultbyEncode.txt");
  // get size of file
  getline(infile, encoded);

  result_size = encoded.length() / 4 * 3;
  result_buf = new char[result_size];

  base64d(encoded.c_str(), -1, result_buf, &result_size);
  
  
  ofstream oOut2("resultbyDecode.jpg", ofstream::binary);
  oOut2.write(result_buf, result_size);
  oOut2.close();
  delete result_buf;
  return 0;


}
