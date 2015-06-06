#include <stdio.h>

//img1 : from DB
//img2 : by VIMG
int DataComp(const unsigned char* img1, const unsigned char* img2)
{
  FILE* fp;
  char buf[100];
  printf("DataComp %x %x\n", img1, img2);
  if(fp = fopen("DataComp_ret.txt", "r")){
    fgets(buf, 100, fp);
    fclose(fp);
    return atoi(buf, 10);
  }
  return 0;
}
