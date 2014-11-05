#include "camerastill.h"
#include <iostream>
#include <fstream>

using namespace std;
int main()
{
  CameraStill* m_cameraStill;

  m_cameraStill = new CameraStill(10); //10 sec
  char* imgBuf = NULL;;
  int imgLength;
  if(m_cameraStill->takePicture(&imgBuf, &imgLength, 2))  //2sec 
  {
    //for test
    if(imgLength < 0)
      cout << "takePicture error!" << endl;
    else{
      ofstream outfile ("test.jpg",ofstream::binary);
      outfile.write (imgBuf,imgLength);
      outfile.close();
    }
  }

  while(1)
    sleep(1);
  
  return 0;
}

