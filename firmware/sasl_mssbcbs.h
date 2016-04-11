// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SASL_MSSBCBS_H
#define SASL_MSSBCBS_H

#include "azure_uamqp_c/sasl_mechanism.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	extern CONCRETE_SASL_MECHANISM_HANDLE saslmssbcbs_create(void* config);
	extern void saslmssbcbs_destroy(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism_concrete_handle);
	extern int saslmssbcbs_get_init_bytes(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism_concrete_handle, SASL_MECHANISM_BYTES* init_bytes);
	extern const char* saslmssbcbs_get_mechanism_name(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism);
	extern int saslmssbcbs_challenge(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism, const SASL_MECHANISM_BYTES* challenge_bytes, SASL_MECHANISM_BYTES* response_bytes);
	extern const SASL_MECHANISM_INTERFACE_DESCRIPTION* saslmssbcbs_get_interface(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SASL_MSSBCBS_H */
