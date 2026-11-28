#ifndef FIRMWARE_UPDATE_H
#define FIRMWARE_UPDATE_H

#include <stdint.h>
#include <stddef.h>

#include "protocol_manager.h"
#include "status.h"

status_t firmware_update_init(const protocol_transport_t *transport);
status_t firmware_update_process(void);


#endif // FIRMWARE_UPDATE_H