/*
 * E131_interface.cpp
 *
 *  Created on: Apr 27, 2017
 *      Author: erakhorst
 */


#include "E131_interface.h"
#include "E131.h"

extern "C"
{
    E131Handle create_E131() { return new E131(); }
    void    free_E131(E131Handle p) { delete p; }
    uint16_t E131_parsePacket(E131Handle p) { return p->parsePacket(); };
    uint8_t * E131_getBuffer(E131Handle p) { return p->packet->raw;};
    uint8_t * E131_getData(E131Handle p) { return p->data;};
    uint16_t E131_getUniverse(E131Handle p) { return p->universe;};

}

