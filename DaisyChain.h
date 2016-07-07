/*
 * DaisyChain.h
 *
 *  Created on: Jul 6, 2016
 *      Author: faebsn
 */

#ifndef DAISYCHAIN_H_
#define DAISYCHAIN_H_

#include "min.h"

void initDaisy(void);

void DaisyChainSendData(uint8_t address,uint8_t length,uint8_t* data);

#endif /* DAISYCHAIN_H_ */
