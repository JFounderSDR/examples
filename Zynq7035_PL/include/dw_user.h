/**
 * \file headless.h
 *
 * \brief Contains definitions for headless.c
*/

/**
* \page Disclaimer Legal Disclaimer
* Copyright 2015-2017 Analog Devices Inc.
* Released under the AD9371 API license, for more information see the "LICENSE.txt" file in this zip file.
*
*/

#ifndef HEADLESS_H_
#define HEADLESS_H_

#include "t_dwrf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 ****************************************************************************
 * General user api functions: <<user-api-guide.pdf>>
 ****************************************************************************
 */

dwApiErr_t dw_initDevice(void *extrnal);
dwApiErr_t dw_setRfEnable(dwRfChannels_t chSelect);
dwApiErr_t dw_setRfDisable(dwRfChannels_t chSelect);
dwApiErr_t dw_setRfPllFrequency(dwRfPllName_t pllName, double rfPllLoFrequency_MHz);
dwApiErr_t dw_setTxAttenuation(dwRfChannels_t tx1tx2, double Attenuation_dB);
dwApiErr_t dw_setRxGainControlMode(dwRxGainMode_t mode);
dwApiErr_t dw_setRxMGCGain(dwRfChannels_t rx1rx2, double Gain_dB);
dwApiErr_t dw_setClcokSource(dwClockSorce_t clkSource);
dwApiErr_t dw_setClockOutput(dwOutputClkSelect_t outputClkSelect);
dwApiErr_t dw_initCalibration(void);

#ifdef __cplusplus
}
#endif

#endif /* HEADLESS_H_ */
