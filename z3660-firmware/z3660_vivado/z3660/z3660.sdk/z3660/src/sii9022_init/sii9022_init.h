/*
 * sii9022_init.h
 *
 *  Created on: 2017Äê8ÔÂ24ÈÕ
 *      Author: pgsimple
 */

#ifndef SRC_SII9022_INIT_H_
#define SRC_SII9022_INIT_H_

#include "xil_types.h"
#include "../rtg/zz_video_modes.h"

int iic_write_8(uint8_t a,uint8_t data);
int iic_master_init(void);
int sii9022_init(zz_video_mode *vmode);

/* Audio  */
#define SII902X_TPI_I2S_ENABLE_MAPPING_REG    0x1f
#define SII902X_TPI_I2S_CONFIG_FIFO0           (0 << 0)
#define SII902X_TPI_I2S_CONFIG_FIFO1           (1 << 0)
#define SII902X_TPI_I2S_CONFIG_FIFO2           (2 << 0)
#define SII902X_TPI_I2S_CONFIG_FIFO3           (3 << 0)
#define SII902X_TPI_I2S_LEFT_RIGHT_SWAP        (1 << 2)
#define SII902X_TPI_I2S_AUTO_DOWNSAMPLE        (1 << 3)
#define SII902X_TPI_I2S_SELECT_SD0             (0 << 4)
#define SII902X_TPI_I2S_SELECT_SD1             (1 << 4)
#define SII902X_TPI_I2S_SELECT_SD2             (2 << 4)
#define SII902X_TPI_I2S_SELECT_SD3             (3 << 4)
#define SII902X_TPI_I2S_FIFO_ENABLE            (1 << 7)

#define SII902X_TPI_I2S_INPUT_CONFIG_REG      0x20
#define SII902X_TPI_I2S_FIRST_BIT_SHIFT_YES    (0 << 0)
#define SII902X_TPI_I2S_FIRST_BIT_SHIFT_NO     (1 << 0)
#define SII902X_TPI_I2S_SD_DIRECTION_MSB_FIRST (0 << 1)
#define SII902X_TPI_I2S_SD_DIRECTION_LSB_FIRST (1 << 1)
#define SII902X_TPI_I2S_SD_JUSTIFY_LEFT        (0 << 2)
#define SII902X_TPI_I2S_SD_JUSTIFY_RIGHT       (1 << 2)
#define SII902X_TPI_I2S_WS_POLARITY_LOW        (0 << 3)
#define SII902X_TPI_I2S_WS_POLARITY_HIGH       (1 << 3)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_128    (0 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_256    (1 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_384    (2 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_512    (3 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_768    (4 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_1024   (5 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_1152   (6 << 4)
#define SII902X_TPI_I2S_MCLK_MULTIPLIER_192    (7 << 4)
#define SII902X_TPI_I2S_SCK_EDGE_FALLING       (0 << 7)
#define SII902X_TPI_I2S_SCK_EDGE_RISING        (1 << 7)

#define SII902X_TPI_I2S_STRM_HDR_BASE    0x21
#define SII902X_TPI_I2S_STRM_HDR_SIZE    5

#define SII902X_TPI_AUDIO_CONFIG_BYTE2_REG    0x26
#define SII902X_TPI_AUDIO_CODING_STREAM_HEADER (0 << 0)
#define SII902X_TPI_AUDIO_CODING_PCM           (1 << 0)
#define SII902X_TPI_AUDIO_CODING_AC3           (2 << 0)
#define SII902X_TPI_AUDIO_CODING_MPEG1         (3 << 0)
#define SII902X_TPI_AUDIO_CODING_MP3           (4 << 0)
#define SII902X_TPI_AUDIO_CODING_MPEG2         (5 << 0)
#define SII902X_TPI_AUDIO_CODING_AAC           (6 << 0)
#define SII902X_TPI_AUDIO_CODING_DTS           (7 << 0)
#define SII902X_TPI_AUDIO_CODING_ATRAC         (8 << 0)
#define SII902X_TPI_AUDIO_MUTE_DISABLE         (0 << 4)
#define SII902X_TPI_AUDIO_MUTE_ENABLE          (1 << 4)
#define SII902X_TPI_AUDIO_LAYOUT_2_CHANNELS    (0 << 5)
#define SII902X_TPI_AUDIO_LAYOUT_8_CHANNELS    (1 << 5)
#define SII902X_TPI_AUDIO_INTERFACE_DISABLE    (0 << 6)
#define SII902X_TPI_AUDIO_INTERFACE_SPDIF      (1 << 6)
#define SII902X_TPI_AUDIO_INTERFACE_I2S        (2 << 6)

#define SII902X_TPI_AUDIO_CONFIG_BYTE3_REG    0x27
#define SII902X_TPI_AUDIO_FREQ_STREAM          (0 << 3)
#define SII902X_TPI_AUDIO_FREQ_32KHZ           (1 << 3)
#define SII902X_TPI_AUDIO_FREQ_44KHZ           (2 << 3)
#define SII902X_TPI_AUDIO_FREQ_48KHZ           (3 << 3)
#define SII902X_TPI_AUDIO_FREQ_88KHZ           (4 << 3)
#define SII902X_TPI_AUDIO_FREQ_96KHZ           (5 << 3)
#define SII902X_TPI_AUDIO_FREQ_176KHZ          (6 << 3)
#define SII902X_TPI_AUDIO_FREQ_192KHZ          (7 << 3)
#define SII902X_TPI_AUDIO_SAMPLE_SIZE_STREAM   (0 << 6)
#define SII902X_TPI_AUDIO_SAMPLE_SIZE_16       (1 << 6)
#define SII902X_TPI_AUDIO_SAMPLE_SIZE_20       (2 << 6)
#define SII902X_TPI_AUDIO_SAMPLE_SIZE_24       (3 << 6)

/* Indirect internal register access */
#define SII902X_IND_SET_PAGE        0xbc
#define SII902X_IND_OFFSET          0xbd
#define SII902X_IND_VALUE           0xbe

#endif /* SRC_SII9022_INIT_H_ */
