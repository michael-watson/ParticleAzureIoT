// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef MESSAGING_H
#define MESSAGING_H

#include "amqpvalue.h"
#include "amqp_definitions.h"

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif /* __cplusplus */

	extern AMQP_VALUE messaging_create_source(const char* address);
	extern AMQP_VALUE messaging_create_target(const char* address);

	extern AMQP_VALUE messaging_delivery_received(uint32_t section_number, uint64_t section_offset);
	extern AMQP_VALUE messaging_delivery_accepted(void);
	extern AMQP_VALUE messaging_delivery_rejected(const char* error_condition, const char* error_description);
	extern AMQP_VALUE messaging_delivery_released(void);
	extern AMQP_VALUE messaging_delivery_modified(bool delivery_failed, bool undeliverable_here, fields message_annotations);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MESSAGING_H */
