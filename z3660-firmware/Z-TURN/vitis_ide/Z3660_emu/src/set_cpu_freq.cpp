#include "xil_io.h"
#include <stdio.h>
#include "xil_cache.h"
#include "xil_cache_l.h"
#include "xil_misc_psreset_api.h"

// SLCR registers for Zynq-7020
#define XSLCR_PLL_STATUS      (XSLCR_BASEADDR + 0x164)
#define XSLCR_LOCK_ADDR       (XSLCR_BASEADDR + 0x004)  // Lock register

// Keys to unlock/lock SLCR
#define SLCR_UNLOCK_KEY       0xDF0D
#define SLCR_LOCK_KEY         0x767B

/**
 * @brief Unlocks SLCR to allow writes
 */
void slcr_unlock() {
    Xil_Out32(XSLCR_LOCK_ADDR, SLCR_UNLOCK_KEY);
    // Memory barrier and wait
    __asm__ volatile ("dsb sy");
    __asm__ volatile ("isb sy");
    for(volatile int i = 0; i < 100; i++);
    printf("SLCR unlocked - LOCK: 0x%08lX\n", Xil_In32(XSLCR_LOCK_ADDR));
}

/**
 * @brief Locks SLCR after modifications
 */
void slcr_lock() {
    Xil_Out32(XSLCR_LOCK_ADDR, SLCR_LOCK_KEY);
    // Memory barrier
    __asm__ volatile ("dsb sy");
    __asm__ volatile ("isb sy");
    for(volatile int i = 0; i < 100; i++);
}

/**
 * @brief Configures CPU frequency in Zynq-7020 (CORRECTED VERSION)
 */
int set_cpu_frequency(uint32_t target_freq_mhz) {
    const uint32_t INPUT_FREQ_MHZ = 33; // PLL input frequency
    
    // Validate target frequency
    if (target_freq_mhz < 100 || target_freq_mhz > 1300) {
        printf("Target frequency out of range (100-1300 MHz)\n");
        return -1;
    }
    
    // Calculate best FBDIV and divisor values
    uint32_t best_divisor = 1;
    uint32_t best_fbdiv = 0;
    uint32_t best_error = 0xFFFFFFFF;
    
    // Try different divisors (1, 2, 3, 4)
    for (uint32_t divisor = 1; divisor <= 4; divisor++) {
        uint32_t required_fbdiv = (target_freq_mhz * divisor + INPUT_FREQ_MHZ / 2) / INPUT_FREQ_MHZ;
        
        if (required_fbdiv >= 16 && required_fbdiv <= 80) {
            uint32_t actual_freq = (INPUT_FREQ_MHZ * required_fbdiv) / divisor;
            uint32_t error = (actual_freq > target_freq_mhz) ? 
                           (actual_freq - target_freq_mhz) : (target_freq_mhz - actual_freq);
            
            if (error < best_error) {
                best_error = error;
                best_divisor = divisor;
                best_fbdiv = required_fbdiv;
            }
        }
    }
    
    if (best_fbdiv == 0) {
        printf("No valid values found for FBDIV and Divisor\n");
        return -1;
    }
    
    // 1. UNLOCK SLCR
    slcr_unlock();
    
    // 2. Read current configuration and display
    uint32_t original_pll = Xil_In32(XSLCR_ARM_PLL_CTRL_ADDR);
    uint32_t original_clk = Xil_In32(XSLCR_ARM_CLK_CTRL_ADDR);
    printf("Original configuration - PLL: 0x%08lX, CLK: 0x%08lX\n", original_pll, original_clk);
    printf("Original FBDIV: %lu, Original Divisor: %lu\n", 
           (original_pll >> 12) & 0x7F, ((original_clk >> 8) & 0x3F) + 1);
    
    // 3. FIRST configure new FBDIV (bits 12-18)
    uint32_t new_pll = (original_pll & 0xFFFFE00F) | (best_fbdiv << 12);
    Xil_Out32(XSLCR_ARM_PLL_CTRL_ADDR, new_pll);
    __asm__ volatile ("dsb sy");
    for(volatile int i = 0; i < 10000; i++);
    
    // 4. Configure divisor (bits 8-9)
    uint32_t new_clk = (original_clk & 0xFFFFFCFF) | ((best_divisor - 1) << 8);
    Xil_Out32(XSLCR_ARM_CLK_CTRL_ADDR, new_clk);
    __asm__ volatile ("dsb sy");
    for(volatile int i = 0; i < 10000; i++);
    
    // 5. Put PLL in BYPASS (bit 4 = 1)
    Xil_Out32(XSLCR_ARM_PLL_CTRL_ADDR, new_pll | 0x10); // BYPASS_FORCE=1
    __asm__ volatile ("dsb sy");
    for(volatile int i = 0; i < 10000; i++);
    
    // 6. Hacer RESET del PLL (bit 0 = 1) - PASO CRÍTICO
    Xil_Out32(XSLCR_ARM_PLL_CTRL_ADDR, new_pll | 0x11); // BYPASS=1 + RESET=1
    __asm__ volatile ("dsb sy");
    for(volatile int i = 0; i < 10000; i++);
    
    // 7. Remove RESET (bit 0 = 0)
    Xil_Out32(XSLCR_ARM_PLL_CTRL_ADDR, new_pll | 0x10); // BYPASS=1 + RESET=0
    __asm__ volatile ("dsb sy");
    for(volatile int i = 0; i < 10000; i++);
    
    // 8. Wait for PLL to LOCK
    int timeout = 1000000;
    int pll_locked = 0;
    for(volatile int i = 0; i < timeout; i++) {
        if (Xil_In32(XSLCR_PLL_STATUS) & 0x1) {
            pll_locked = 1;
            break;
        }
    }
    
    if (!pll_locked) {
        printf("ERROR: PLL did not lock\n");
        slcr_lock();
        return -1;
    }
    
    // 9. Remove BYPASS (bit 4 = 0) to use the PLL
    Xil_Out32(XSLCR_ARM_PLL_CTRL_ADDR, new_pll & 0xFFFFFFEF); // BYPASS_FORCE=0
    __asm__ volatile ("dsb sy");
    for(volatile int i = 0; i < 10000; i++);
    
    printf("PLL locked and bypass removed successfully!\n");
    
    // 10. LOCK SLCR
    slcr_lock();
    
    // Final stabilization wait
    for(volatile int i = 0; i < 10000; i++);
    printf("CPU frequency set to approximately %lu MHz (FBDIV=%lu, Divisor=%lu)\n", 
           (INPUT_FREQ_MHZ * best_fbdiv) / best_divisor, best_fbdiv, best_divisor);
    
    // Read and display final configuration for verification
    uint32_t final_pll = Xil_In32(XSLCR_ARM_PLL_CTRL_ADDR);
    uint32_t final_clk = Xil_In32(XSLCR_ARM_CLK_CTRL_ADDR);
    printf("Final configuration - PLL: 0x%08lX, CLK: 0x%08lX\n", final_pll, final_clk);
    
    // IMPORTANT: Flush caches after changing frequency
    Xil_L1DCacheFlush();
    Xil_L2CacheFlush();
    Xil_DCacheFlush(); 
    Xil_ICacheInvalidate();
    
    // Additional small wait
    for(volatile int i = 0; i < 10000; i++);
    
    return 0;
}

/**
 * @brief Gets current configured CPU frequency
 * @return Frequency in Hz, 0 if error
 */
uint32_t get_current_cpu_frequency() {
#define INPUT_FREQ_HZ 33.333333e6f
    
    uint32_t pll_ctrl = Xil_In32(XSLCR_ARM_PLL_CTRL_ADDR);
    uint32_t clk_ctrl = Xil_In32(XSLCR_ARM_CLK_CTRL_ADDR);
    
    uint32_t fbdiv = (pll_ctrl >> 12) & 0x7F;
    uint32_t divisor = (clk_ctrl >> 8) & 0x3F;
    
    if (fbdiv < 16 || fbdiv > 80 || divisor < 1 || divisor > 4) {
        printf("Invalid PLL configuration (FBDIV=%lu, Divisor=%lu)\n", fbdiv, divisor);
        return 0; // Invalid values
    }
    
    return (INPUT_FREQ_HZ * fbdiv) / divisor;
}

/**
 * @brief Simple version for common configurations
 * @param freq_code Frequency code: 0=667MHz, 1=767MHz, 2=867MHz, 3=900MHz, 4=933MHz, 5=967MHz, 6=1000MHz,
 *                  7=1033MHz, 8=1067MHz, 9=1100MHz, 10=1133MHz, 11=1167MHz, 12=1200MHz,
 *                  13=1233MHz, 14=1267MHz, 15=1300MHz
 * @return 0 if success, -1 if error
 */
int set_cpu_frequency_simple(int freq_code) {
    switch(freq_code) {
        case 0: return set_cpu_frequency(667);   // 667 MHz
        case 1: return set_cpu_frequency(767);   // 767 MHz  
        case 2: return set_cpu_frequency(867);   // 867 MHz
        case 3: return set_cpu_frequency(900);   // 900 MHz
        case 4: return set_cpu_frequency(933);   // 933 MHz
        case 5: return set_cpu_frequency(967);   // 967 MHz
        case 6: return set_cpu_frequency(1000);  // 1000 MHz
        case 7: return set_cpu_frequency(1033);  // 1033 MHz
        case 8: return set_cpu_frequency(1067);  // 1067 MHz
        case 9: return set_cpu_frequency(1100);  // 1100 MHz
        case 10: return set_cpu_frequency(1133); // 1133 MHz
        case 11: return set_cpu_frequency(1167); // 1167 MHz
        case 12: return set_cpu_frequency(1200); // 1200 MHz
        case 13: return set_cpu_frequency(1233); // 1233 MHz
        case 14: return set_cpu_frequency(1267); // 1267 MHz
        case 15: return set_cpu_frequency(1300); // 1300 MHz
        default: return -1;
    }
}
