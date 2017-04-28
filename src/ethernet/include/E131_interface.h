/*
 * E131_interface.h
 *
 *  Created on: Apr 27, 2017
 *      Author: erakhorst
 */

#ifndef ETHERNET_INCLUDE_E131_INTERFACE_H_
#define ETHERNET_INCLUDE_E131_INTERFACE_H_

#include <sys/types.h>

typedef struct E131 E131;

#ifdef __cplusplus
extern "C" {
#endif


typedef E131 * E131Handle;

#define E131_PACKET_SIZE 638


E131Handle create_E131();
void free_E131(E131Handle p);
uint16_t E131_parsePacket(E131Handle p);

uint8_t * E131_getBuffer(E131Handle p);
uint8_t * E131_getData(E131Handle p);
uint16_t E131_getUniverse(E131Handle p);



#ifdef __cplusplus
}
#endif


#endif /* ETHERNET_INCLUDE_E131_INTERFACE_H_ */
