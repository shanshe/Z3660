//#include <memory>
//#include <vector>
//#include "bitfile.h"

#include "jtag.h"


void detect_chain(Jtag *j,IOBase *io);
int getIO(IOBase *io, const char *serial, bool verbose, unsigned int freq);
const char *getCableName(int type);

#define OSNAME_LEN	64
void get_os_name(char *buf, int buflen);


/* Split string on delimiting character. */
char * splitString(char *s, char delim);


int idToIRLength(DeviceID idcode);
int idToIDCmd(DeviceID idcode);
char *idToDescription(DeviceID idcode);
