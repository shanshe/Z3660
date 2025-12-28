#include "xil_io.h"
#include "ps7_init_simple.h"
#include <stdio.h>

uint32_t get_current_cpu_frequency();

// Macros exactly as in Xilinx FSBL
#define EMIT_WRITE(addr, val) do { \
    Xil_Out32((addr), (val)); \
    __asm__ volatile ("dsb st"); \
    __asm__ volatile ("isb sy"); \
} while (0)

#define EMIT_MASKWRITE(addr, mask, val) do { \
    uint32_t reg_val = Xil_In32(addr); \
    reg_val = (reg_val & (~(mask))) | (val); \
    Xil_Out32((addr), reg_val); \
    __asm__ volatile ("dsb st"); \
    __asm__ volatile ("isb sy"); \
} while (0)

#define EMIT_MASKPOLL(addr, mask) do { \
    int timeout = 1000000; \
    int locked = 0; \
    for(volatile int i = 0; i < timeout; i++) { \
        if (Xil_In32(addr) & (mask)) { \
            locked = 1; \
            break; \
        } \
    } \
    if (!locked) { \
        printf("ERROR: Timeout waiting for mask 0x%08X at 0x%08X\n", (unsigned int)(mask), (unsigned int)(addr)); \
        return -1; \
    } \
} while (0)

/**
 * @brief Implementation based exactly on Xilinx FSBL
 * Configures ARM PLL for different frequencies according to freq_code
 * @param freq_code 0=667MHz, 1=767MHz, 2=867MHz, 3=900MHz, 4=933MHz, 5=967MHz, 6=1000MHz,
 *                  7=1033MHz, 8=1067MHz, 9=1100MHz, 10=1133MHz, 11=1167MHz, 12=1200MHz,
 *                  13=1233MHz, 14=1267MHz, 15=1300MHz
 */
int ps7_init_custom(int freq_code) {
    uint32_t target_fbdiv, pll_params, pll_ctrl_val;
    const char *freq_name;
    printf("Current CPU Frequency: %3.3f MHz\n",get_current_cpu_frequency()/1000000.0f);

    switch(freq_code) {
        case 0: // 667 MHz (FBDIV=40, Divisor=2)
            target_fbdiv = 40;
            pll_params = 0x000FA220U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x2
            pll_ctrl_val = 0x00028000U;
            freq_name = "667 MHz";
            break;
        case 1: // 767 MHz (FBDIV=46, Divisor=2) 
            target_fbdiv = 46;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0002E000U;
            freq_name = "767 MHz";
            break;
        case 2: // 867 MHz (FBDIV=52, Divisor=2)
            target_fbdiv = 52;
            pll_params = 0x000FA240U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x4
            pll_ctrl_val = 0x00034000U;
            freq_name = "867 MHz";
            break;
        case 3: // 900 MHz (FBDIV=54, Divisor=2)
            target_fbdiv = 54;
            pll_params = 0x000FA240U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x4
            pll_ctrl_val = 0x00036000U;
            freq_name = "900 MHz";
            break;
        case 4: // 933 MHz (FBDIV=56, Divisor=2)
            target_fbdiv = 56;
            pll_params = 0x000FA220U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x2
            pll_ctrl_val = 0x00038000U;
            freq_name = "933 MHz";
            break;
        case 5: // 967 MHz (FBDIV=58, Divisor=2)
            target_fbdiv = 58;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0003A000U;
            freq_name = "967 MHz";
            break;
        case 6: // 1000 MHz (FBDIV=60, Divisor=2)
            target_fbdiv = 60;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0003C000U;
            freq_name = "1000 MHz";
            break;
        case 7: // 1033 MHz (FBDIV=62, Divisor=2)
            target_fbdiv = 62;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0003E000U;
            freq_name = "1033 MHz";
            break;
        case 8: // 1067 MHz (FBDIV=64, Divisor=2)
            target_fbdiv = 64;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x00040000U;
            freq_name = "1067 MHz";
            break;
        case 9: // 1100 MHz (FBDIV=66, Divisor=2)
            target_fbdiv = 66;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x00042000U;
            freq_name = "1100 MHz";
            break;
        case 10: // 1133 MHz (FBDIV=68, Divisor=2)
            target_fbdiv = 68;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x00044000U;
            freq_name = "1133 MHz";
            break;
        case 11: // 1167 MHz (FBDIV=70, Divisor=2)
            target_fbdiv = 70;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x00046000U;
            freq_name = "1167 MHz";
            break;
        case 12: // 1200 MHz (FBDIV=72, Divisor=2)
            target_fbdiv = 72;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x00048000U;
            freq_name = "1200 MHz";
            break;
        case 13: // 1233 MHz (FBDIV=74, Divisor=2)
            target_fbdiv = 74;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0004A000U;
            freq_name = "1233 MHz";
            break;
        case 14: // 1267 MHz (FBDIV=76, Divisor=2)
            target_fbdiv = 76;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0004C000U;
            freq_name = "1267 MHz";
            break;
        case 15: // 1300 MHz (FBDIV=78, Divisor=2)
            target_fbdiv = 78;
            pll_params = 0x000FA260U;  // LOCK_CNT=0xFA, PLL_CP=0x2, PLL_RES=0x6
            pll_ctrl_val = 0x0004E000U;
            freq_name = "1300 MHz";
            break;
        default:
            printf("[Core0] Invalid frequency code: %d\n", freq_code);
            return -1;
    }
    
    printf("ps7_init_custom: Configuring ARM PLL for %s (FBDIV=%lu)...\n", freq_name, target_fbdiv);
    
    // 1. UNLOCK SLCR (same as FSBL)
    EMIT_WRITE(0XF8000008, 0x0000DF0DU);
//    printf("SLCR unlocked\n");
    
    // 2. Configure PLL parameters
    EMIT_MASKWRITE(0XF8000110, 0x003FFFF0U, pll_params);
//    printf("PLL parameters configured\n");
    
    // 3. Configure FBDIV
    EMIT_MASKWRITE(0XF8000100, 0x0007F000U, pll_ctrl_val);
//    printf("FBDIV=%lu configured for %s\n", target_fbdiv, freq_name);
    
    // 4. Put PLL in bypass
    EMIT_MASKWRITE(0XF8000100, 0x00000010U, 0x00000010U);
//    printf("PLL in bypass\n");
    
    // 5. Reset PLL  
    EMIT_MASKWRITE(0XF8000100, 0x00000001U, 0x00000001U);
//    printf("PLL reset activated\n");
    
    // 6. Remove PLL reset
    EMIT_MASKWRITE(0XF8000100, 0x00000001U, 0x00000000U);
//    printf("PLL reset deactivated\n");
    
    // 7. Wait for PLL to lock
    EMIT_MASKPOLL(0XF800010C, 0x00000001U);
//    printf("PLL locked\n");
    
    // 8. Remove PLL bypass
    EMIT_MASKWRITE(0XF8000100, 0x00000010U, 0x00000000U);
//    printf("PLL bypass removed\n");
    
    // 9. Configure clock divisor (Divisor=2)
    EMIT_MASKWRITE(0XF8000120, 0x1F003F30U, 0x1F000200U);
//    printf("Divisor=2 configured\n");
    
    printf("ps7_init_custom: ARM PLL successfully configured for %s\n", freq_name);

    ps7_ddr_get_frequency(1);

    return 0;
}

// Original function for compatibility
int ps7_init() {
    return ps7_init_custom(0); // Default 667 MHz
}



/**
 * @brief Reads current DDR PLL frequency configuration
 * @param debug Set to 1 to print debug information, 0 for silent
 * @return DDR frequency in MHz, or 0 if unable to read
 */
int ps7_ddr_get_frequency(int debug) {
    // Leer el registro de configuración del PLL DDR (0xF8000104)
    uint32_t ddr_pll_ctrl = Xil_In32(0xF8000104);
    
    // Extraer el valor FBDIV (bits [18:12])
    uint32_t fbdiv = (ddr_pll_ctrl >> 12) & 0x7F;
    
    // Leer registro de divisores DDR (0xF8000124)
    uint32_t ddr_divisors = Xil_In32(0xF8000124);
    uint32_t ddr3x_divisor = (ddr_divisors >> 20) & 0x7;
    uint32_t ddr2x_divisor = (ddr_divisors >> 26) & 0x7;
    
    // Leer si el PLL está en bypass
    uint32_t bypass = (ddr_pll_ctrl >> 4) & 0x1;
    uint32_t reset = ddr_pll_ctrl & 0x1;
    
    if (debug) {
        printf("DDR PLL Debug:\n");
        printf("  FBDIV=0x%lx (%lu)\n", fbdiv, fbdiv);
        printf("  DDR3X_DIV=0x%lx, DDR2X_DIV=0x%lx\n", ddr3x_divisor, ddr2x_divisor);
        printf("  BYPASS=%lu, RESET=%lu\n", bypass, reset);
        printf("  PLL_CTRL register: 0x%08lx\n", ddr_pll_ctrl);
        printf("  DIVISORS register: 0x%08lx\n", ddr_divisors);
    }
    
    // Si el PLL está en bypass o reset, la frecuencia no es válida
    if (bypass || reset) {
        if (debug) printf("  DDR PLL not configured properly (bypass or reset)\n");
        return 0;
    }
    
    // Calcular frecuencia base del PLL (FBDIV * 33.333MHz)
    uint32_t pll_freq_mhz = fbdiv * 33333 / 1000;
    
    // El clock DDR real se divide por DDR3X divisor (típicamente 2)
    // DDR frecuencia = pll_freq_mhz / DDR3X_DIV
    uint32_t ddr_freq_mhz = pll_freq_mhz / ddr3x_divisor;
    
    if (debug) {
        printf("  Calculated: PLL=%lu MHz, DDR=%lu MHz\n", pll_freq_mhz, ddr_freq_mhz);
    }
    
    // Redondear al múltiplo más cercano para valores estándar
    if (ddr_freq_mhz >= 780 && ddr_freq_mhz <= 820) {
        if (debug) printf("  Detected: 800 MHz DDR\n");
        return 800; // 800 MHz
    } else if (ddr_freq_mhz >= 650 && ddr_freq_mhz <= 680) {
        if (debug) printf("  Detected: 667 MHz DDR\n");
        return 667; // 667 MHz
    } else if (ddr_freq_mhz >= 520 && ddr_freq_mhz <= 550) {
        if (debug) printf("  Detected: 533 MHz DDR\n");
        return 533; // 533 MHz
    } else {
        if (debug) printf("  Detected: %lu MHz DDR (non-standard)\n", ddr_freq_mhz);
        return ddr_freq_mhz; // Retornar valor calculado si no coincide con valores estándar
    }
}

/**
 * @brief Simple wrapper for backward compatibility
 */
int ps7_ddr_get_frequency_simple(void) {
    return ps7_ddr_get_frequency(0);
}

/**
 * @brief Reads DDR PLL configuration details
 * @param fbdiv Pointer to store FBDIV value (optional, can be NULL)
 * @param divisors Pointer to store divisors value (optional, can be NULL)
 * @return 0 if success, -1 if error
 */
int ps7_ddr_get_config(uint32_t *fbdiv, uint32_t *divisors) {
    // Leer registro de control del PLL DDR
    uint32_t ddr_pll_ctrl = Xil_In32(0xF8000104);
    
    // Extraer FBDIV
    uint32_t read_fbdiv = (ddr_pll_ctrl >> 12) & 0x7F;
    
    // Leer registro de divisores DDR
    uint32_t ddr_divisors = Xil_In32(0xF8000124);
    uint32_t ddr3x_divisor = (ddr_divisors >> 20) & 0x7;
    uint32_t ddr2x_divisor = (ddr_divisors >> 26) & 0x7;
    uint32_t read_divisors = (ddr2x_divisor << 8) | (ddr3x_divisor << 4);
    
    if (fbdiv) {
        *fbdiv = read_fbdiv;
    }
    if (divisors) {
        *divisors = read_divisors;
    }
    
    return 0;
}

/**
 * @brief Simple function to get silicon version
 */
int ps7GetSiliconVersion () {
  // Read PS version from MCTRL register [31:28]
  unsigned long mask = 0xF0000000;
  unsigned long *addr = (unsigned long*) 0XF8007080;
  int ps_version = (*addr & mask) >> 28;
  return ps_version;
}
