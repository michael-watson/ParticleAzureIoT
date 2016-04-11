// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MESSAGE_H
#define MESSAGE_H

#include "amqpvalue.h"
#include "amqp_definitions.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#else
#include <stddef.h>
#endif /* __cplusplus */

	typedef enum MESSAGE_BODY_TYPE_TAG
	{
		MESSAGE_BODY_TYPE_NONE,
		MESSAGE_BODY_TYPE_DATA,
		MESSAGE_BODY_TYPE_SEQUENCE,
		MESSAGE_BODY_TYPE_VALUE
	} MESSAGE_BODY_TYPE;

	typedef struct MESSAGE_INSTANCE_TAG* MESSAGE_HANDLE;
	typedef struct BINARY_DATA_TAG
	{
		const unsigned char* bytes;
		size_t length;
	} BINARY_DATA;

	extern MESSAGE_HANDLE message_create(void);
	extern MESSAGE_HANDLE message_clone(MESSAGE_HANDLE source_message);
	extern void message_destroy(MESSAGE_HANDLE message);
	extern int message_set_header(MESSAGE_HANDLE message, HEADER_HANDLE message_header);
	extern int message_get_header(MESSAGE_HANDLE message, HEADER_HANDLE* message_header);
	extern int message_set_delivery_annotations(MESSAGE_HANDLE message, annotations delivery_annotations);
	extern int message_get_delivery_annotations(MESSAGE_HANDLE message, annotations* delivery_annotations);
	extern int message_set_message_annotations(MESSAGE_HANDLE message, annotations delivery_annotations);
	extern int message_get_message_annotations(MESSAGE_HANDLE message, annotations* delivery_annotations);
	extern int message_set_properties(MESSAGE_HANDLE message, PROPERTIES_HANDLE properties);
	extern int message_get_properties(MESSAGE_HANDLE message, PROPERTIES_HANDLE* properties);
	extern int message_set_application_properties(MESSAGE_HANDLE message, AMQP_VALUE application_properties);
	extern int message_get_application_properties(MESSAGE_HANDLE message, AMQP_VALUE* application_properties);
	extern int message_set_footer(MESSAGE_HANDLE message, annotations footer);
	extern int message_get_footer(MESSAGE_HANDLE message, annotations* footer);
	extern int message_add_body_amqp_data(MESSAGE_HANDLE message, BINARY_DATA binary_data);
	extern int message_get_body_amqp_data(MESSAGE_HANDLE message, size_t index, BINARY_DATA* binary_data);
	extern int message_get_body_amqp_data_count(MESSAGE_HANDLE message, size_t* count);
	extern int message_set_body_amqp_value(MESSAGE_HANDLE message, AMQP_VALUE body_amqp_value);
	extern int message_get_inplace_body_amqp_value(MESSAGE_HANDLE message, AMQP_VALUE* body_amqp_value);
	extern int message_get_body_type(MESSAGE_HANDLE message, MESSAGE_BODY_TYPE* body_type);
	extern int message_add_body_amqp_sequence(MESSAGE_HANDLE message, AMQP_VALUE sequence_list);
	extern int message_get_body_amqp_sequence(MESSAGE_HANDLE message, size_t index, AMQP_VALUE* sequence_list);
	extern int message_get_body_amqp_sequence_count(MESSAGE_HANDLE message, size_t* count);
    extern int message_set_message_format(MESSAGE_HANDLE message, uint32_t message_format);
    extern int message_get_message_format(MESSAGE_HANDLE message, uint32_t *message_format);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MESSAGE_H */
