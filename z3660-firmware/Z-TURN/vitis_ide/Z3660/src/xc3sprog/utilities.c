//#ifndef WIN32
//#include <sys/utsname.h>
//#endif

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "iobase.h"
#include "jtag.h"
//#include <memory>

#include "ioparport.h"
//#include "iofx2.h"
//#include "ioftdi.h"
//#include "ioxpc.h"
#include "utilities.h"
#include "iobase.h"
#include <string.h>
#define false 0
#define true 1

extern char *optarg;
#if 0
void detect_chain(Jtag *j,IOBase *io)
{
   int num=getChain(j,io, true);
   for(int i=0; i<num; i++)
   {
      DeviceID id = getDeviceID(j,i);
      printf("JTAG loc.: %3d  IDCODE: 0x%08lx  ", i, (unsigned long)id);
      int length = idToIRLength(id);
      if (length > 0)
      {
         setDeviceIRLength(j,i,length);
         printf("Desc: %s Rev: %c  IR length: %2d\n",
               idToDescription(id),
               (int)(id >> 28) | 'A', length);
      }
      else
      {
         printf("not found in '%s'.\n", "jedec_file");
      }
   }
}
#endif
int  getIO( IOBase *io, char const *serial, bool verbose, unsigned int freq)
{
   int res = 1;
   unsigned int use_freq = 0;
   IOBase_init(io);
//   reset(IOParport());
   setVerbose_io(io,verbose);
   res = Init(use_freq,IOPP_MODE_I2CSW);
   return res;
}

const char *getCableName(int type)
{
   return "pp";
}

