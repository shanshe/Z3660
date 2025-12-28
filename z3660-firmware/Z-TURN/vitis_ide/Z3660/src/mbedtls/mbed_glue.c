#include "mbedtls/platform_time.h"
#include "xtime_l.h"
#include "string.h"
#include "stdlib.h"

extern XTime debug_time_start;

mbedtls_ms_time_t mbedtls_ms_time(void)
{
   XTime_GetTime(&debug_time_start);
   return (mbedtls_ms_time_t)( debug_time_start / 1000);
}
#define MIN(X,Y) (X)>(Y)?(Y):(X)
/* Function to feed mbedtls entropy. */
int mbedtls_hardware_poll(void *data __unused, unsigned char *output, size_t len, size_t *olen) {
   *olen = 0;
   while(*olen < len) {
      uint64_t rand_data = (((uint64_t)rand())<<32)|((uint64_t)rand());
      size_t to_copy = MIN(len, sizeof(rand_data));
      memcpy(output + *olen, &rand_data, to_copy);
      *olen += to_copy;
   }
   return 0;
}
