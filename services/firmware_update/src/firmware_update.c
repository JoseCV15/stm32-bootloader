
#include "firmware_update.h"


static protocol_manager_t update_protocol;


/*
 * This function is called by Protocol Manager
 * whenever a valid new command is received.
 *
 * It does not send ACK/NACK.
 * Protocol Manager handles the response.
 */
static protocol_result_t firmware_update_handle_command(const packet_t *packet, void *context)
{
    (void)context;

    if (packet == NULL) {
        return PROTOCOL_RESULT_INVALID_ARGUMENT;
    }

    switch (packet->command)
    {
        case PACKET_CMD_SYNC:
        {
            if (packet->length != 0U) {
                return PROTOCOL_RESULT_INVALID_ARGUMENT;
            }

            return PROTOCOL_RESULT_OK;
        }

        case PACKET_CMD_GET_INFO:
        {
            /* TODO: Implement bootloader information response */
            return PROTOCOL_RESULT_NOT_IMPLEMENTED;
        }

        case PACKET_CMD_ERASE:
        {
            /* TODO: Validate and erase application Flash */
            return PROTOCOL_RESULT_NOT_IMPLEMENTED;
        }

        case PACKET_CMD_WRITE:
        {
            /* TODO: Program application Flash */
            return PROTOCOL_RESULT_NOT_IMPLEMENTED;
        }

        case PACKET_CMD_VERIFY:
        {
            /* TODO: Verify application image CRC */
            return PROTOCOL_RESULT_NOT_IMPLEMENTED;
        }

        case PACKET_CMD_RESET:
        {
            /* TODO: Validate application and reset MCU */
            return PROTOCOL_RESULT_NOT_IMPLEMENTED;
        }

        default:
        {
            return PROTOCOL_RESULT_UNKNOWN_COMMAND;
        }
    }
}


/* Initialize Firmware Update Service */
status_t firmware_update_init(const protocol_transport_t *transport)
{
    return protocol_manager_init(&update_protocol, transport,
        firmware_update_handle_command, NULL);
}


/* Process communication */
status_t firmware_update_process(void) {
    return protocol_manager_process(&update_protocol);
}
