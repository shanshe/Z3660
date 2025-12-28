#ifndef MOBOTEST_H
#define MOBOTEST_H

void mobotest(int sw1_is_down);
void update_sd(void);

#define CPLD_PROGRAMMING
//#define DIRECT_CPLD_PROGRAMMING
//#define I2C_CPLD_PROGRAMMING
#define I2CSW_CPLD_PROGRAMMING

typedef enum {
   SHANSHE_SERVER,
   GITHUB_SERVER
} server_t;


#endif //MOBOTEST_H
