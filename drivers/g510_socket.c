#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "g510_frt.h"

int g510_socket(){
  if(g510WaitForRegistration(300))
  {
    return -1;
  }
  g510FlushRx();
  g510TxCmd("AT+MIPOPEN?\r");
  char buf[1024];
  g510ReadResp(buf,1024,30000);
  char key[] = "1234567890";
  char * pch = strpbrk (buf, key);
  int i = atoi(pch);
  return i;
}

//bad idea
uint32_t g510_getIP(){
  if(g510WaitForRegistration(300))
  {
    return -1;
  }
  g510FlushRx();
  g510TxCmd("AT+MIPCALL=1,\"" APN "\",\"" USER "\",\"" PASSWD "\"\r");
  char buf[1024];
  g510ReadResp(buf,1024,30000);
  g510ReadResp(buf,1024,30000);
  xprintf("%s\n", buf);

}
