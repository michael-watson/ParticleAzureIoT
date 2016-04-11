// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MESSAGE_RECEIVER_H
#define MESSAGE_RECEIVER_H

#include "link.h"
#include "message.h"
#include "amqp_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef enum MESSAGE_RECEIVER_STATE_TAG
	{
		MESSAGE_RECEIVER_STATE_IDLE,
		MESSAGE_RECEIVER_STATE_OPENING,
		MESSAGE_RECEIVER_STATE_OPEN,
		MESSAGE_RECEIVER_STATE_CLOSING,
		MESSAGE_RECEIVER_STATE_ERROR
	} MESSAGE_RECEIVER_STATE;

	typedef struct MESSAGE_RECEIVER_INSTANCE_TAG* MESSAGE_RECEIVER_HANDLE;
	typedef AMQP_VALUE (*ON_MESSAGE_RECEIVED)(const void* context, MESSAGE_HANDLE message);
	typedef void(*ON_MESSAGE_RECEIVER_STATE_CHANGED)(const void* context, MESSAGE_RECEIVER_STATE new_state, MESSAGE_RECEIVER_STATE previous_state);

	extern MESSAGE_RECEIVER_HANDLE messagereceiver_create(LINK_HANDLE link, ON_MESSAGE_RECEIVER_STATE_CHANGED on_message_receiver_state_changed, void* context);
	extern void messagereceiver_destroy(MESSAGE_RECEIVER_HANDLE message_receiver);
	extern int messagereceiver_open(MESSAGE_RECEIVER_HANDLE message_receiver, ON_MESSAGE_RECEIVED on_message_received, const void* callback_context);
	extern int messagereceiver_close(MESSAGE_RECEIVER_HANDLE message_receiver);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MESSAGE_RECEIVER_H */
