#include <iostream>
#include "maindelegator.h"

using namespace std;

class Listener : public MainDelegator::EventListener{
public:  
  virtual void onMessage(std::string tag, std::string data)
  {
  }
  virtual void onEmployeeInfo(std::string CoName, std::string Name, std::string PinNo)
  {
  }
  virtual void onStatus(std::string status)
  {
  }
};

int main()
{
  cout << "start main\n" << endl;
  Listener li;
  MainDelegator* md = MainDelegator::createInstance(&li);

  while(1){
    cout << "Koong" << endl;
    sleep(100);
  }
  cout << "exit" << endl;
  return 0;
}
