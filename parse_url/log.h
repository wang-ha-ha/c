#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>

#ifdef LOG_ID
#define STR1(R) "[" #R "]"
#define STR2(R) STR1(R)
#define STR3    STR2(LOG_ID)

#define STR11(R) "[ERROR-" #R "]"
#define STR12(R) STR11(R)
#define STR13    STR12(LOG_ID)

#else
#define STR3
#define STR13
#endif


 #define DEBUG_LOG(pmt,...) \
	 do \
     {   \
        printf(STR3 pmt,##__VA_ARGS__); \
     }while(0)

 #define DEBUG_ERR(pmt,...) \
	do \
	{ \
         printf(STR13 "%s:%s:%d " pmt,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);\
    }while(0)


#endif
