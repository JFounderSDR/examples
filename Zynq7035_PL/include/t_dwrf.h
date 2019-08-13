/*
 * t_dwuser.h
 *
 *  Created on: 2018Äê4ÔÂ19ÈÕ
 *      Author: 123
 */

#ifndef T_DWUSER_H_
#define T_DWUSER_H_
#ifdef __cplusplus
extern "C" {
#endif
#ifndef DW_API_ERR
#define DW_API_ERR
typedef enum {DWERR_OK = 0x0, DWERR_INV_PARM, DWERR_FAILED}dwApiErr_t;
#endif

typedef enum {DW_TCXO = 0x0, DW_EXT_CLK = 0x1}dwClockSorce_t;

typedef enum {DW_OC_3072_12288 = 0x0, DW_OC_6144_12288 = 0x1, DW_OC_768_1536 = 0x2, DW_OC_12288_24576 = 0x3, DW_OC_1536_3072 = 0x4, DW_OC_NONE = 0xF}dwOutputClkSelect_t;

typedef enum {DW_RF_TX1 = 0x1, DW_RF_TX2 = 0x2, DW_RF_RX1 = 0x4, DW_RF_RX2 = 0x8}dwRfChannels_t;

typedef enum {DW_RX_PLL = 0x1, DW_TX_PLL, DW_SNIFFER_PLL} dwRfPllName_t;

typedef enum {DW_MGC = 0, DW_AGC = 2, DW_HYBRID = 3} dwRxGainMode_t;
#ifdef __cplusplus
}
#endif
#endif /* T_DWUSER_H_ */
