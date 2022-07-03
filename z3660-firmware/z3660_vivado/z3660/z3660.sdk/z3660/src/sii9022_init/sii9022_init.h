/*
 * sii9022_init.h
 *
 *  Created on: 2017Äê8ÔÂ24ÈÕ
 *      Author: pgsimple
 */

#ifndef SRC_SII9022_INIT_H_
#define SRC_SII9022_INIT_H_

#include "xil_types.h"

int iic_write_8(u8 a,u8 data);
int iic_master_init(void);
int sii9022_init(void);

#endif /* SRC_SII9022_INIT_H_ */
