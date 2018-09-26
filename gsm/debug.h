
#ifndef _DEBUG_
#define _DEBUG_

#include <NeoHWSerial/NeoHWSerial.h>

#define DEBUG false

#if DEBUG

  #define beginDebug()  do { NeoSerial.begin (115200); } while (0)
  #define Tr(x)      NeoSerial.print   (x)
  #define Tr2(x,y)   NeoSerial.print   (x,y)
  #define Trln(x)    NeoSerial.println (x)
  #define Trln2(x,y) NeoSerial.println (x,y)
  #define TrFunc()   do { NeoSerial.print (F("In function: ")); NeoSerial.println (__PRETTY_FUNCTION__); } while (0)


//#else
//  #define beginDebug()  ((void) 0)
//  #define Tr(x)      ((void) 0)
//  #define Tr2(x,y)   ((void) 0)
//  #define Trln(x)    ((void) 0)
//  #define Trln2(x,y) ((void) 0)
//  #define TrFunc()   ((void) 0)
#endif // DEBUG


#endif // _DEBUG_
