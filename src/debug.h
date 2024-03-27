#ifndef _DEBUG_H_
#define _DEBUG_H_

#define DEBUGGING

#ifdef DEBUGGING
#define DEBUG(...) Serial.printf(__VA_ARGS__);
#else
#define DEBUG(...)
#endif

#endif
