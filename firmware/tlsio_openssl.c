// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/crypto.h"
#include <stdio.h>
#include <stdbool.h>
#include "tlsio.h"
#include "tlsio_openssl.h"
#include "socketio.h"
#include "xlogging.h"

typedef enum TLSIO_STATE_TAG
{
    TLSIO_STATE_NOT_OPEN,
    TLSIO_STATE_OPENING_UNDERLYING_IO,
    TLSIO_STATE_IN_HANDSHAKE,
    TLSIO_STATE_OPEN,
    TLSIO_STATE_CLOSING,
    TLSIO_STATE_ERROR
} TLSIO_STATE;

typedef struct TLS_IO_INSTANCE_TAG
{
    XIO_HANDLE underlying_io;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_OPEN_COMPLETE on_io_open_complete;
    ON_IO_CLOSE_COMPLETE on_io_close_complete;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_open_complete_context;
    void* on_io_close_complete_context;
    void* on_io_error_context;
    LOGGER_LOG logger_log;
    SSL* ssl;
    SSL_CTX* ssl_context;
    BIO* in_bio;
    BIO* out_bio;
    TLSIO_STATE tlsio_state;
} TLS_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION tlsio_openssl_interface_description =
{
    tlsio_openssl_create,
    tlsio_openssl_destroy,
    tlsio_openssl_open,
    tlsio_openssl_close,
    tlsio_openssl_send,
    tlsio_openssl_dowork,
    tlsio_openssl_setoption
};

static void indicate_error(TLS_IO_INSTANCE* tls_io_instance)
{
    if (tls_io_instance->on_io_error == NULL)
    {
        LogError("NULL on_io_error.\r\n");

    }
    else
    {
        tls_io_instance->on_io_error(tls_io_instance->on_io_error_context);
    }
}

static void indicate_open_complete(TLS_IO_INSTANCE* tls_io_instance, IO_OPEN_RESULT open_result)
{
    if (tls_io_instance->on_io_open_complete == NULL)
    {
        LogError("NULL on_io_open_complete.\r\n");
    }
    else
    {
        tls_io_instance->on_io_open_complete(tls_io_instance->on_io_open_complete_context, open_result);
    }
}

static int write_outgoing_bytes(TLS_IO_INSTANCE* tls_io_instance, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    int pending = BIO_ctrl_pending(tls_io_instance->out_bio);
    if (pending <= 0)
    {
        result = 0;
    }
    else
    {
        unsigned char* bytes_to_send = malloc(pending);
        if (bytes_to_send == NULL)
        {
            result = __LINE__;
            LogError("NULL bytes_to_send.\r\n");
        }
        else
        {
            if (BIO_read(tls_io_instance->out_bio, bytes_to_send, pending) != pending)
            {
                result = __LINE__;
                LogError("BIO_read not in pending state.\r\n");
            }
            else
            {
                if (xio_send(tls_io_instance->underlying_io, bytes_to_send, pending, on_send_complete, callback_context) != 0)
                {
                    result = __LINE__;
                    LogError("Error in xio_send.\r\n");
                }
                else
                {
                    result = 0;
                }
            }

            free(bytes_to_send);
        }
    }

    return result;
}

static int send_handshake_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result;
    int r = 0;
    int pending = 0;

    if (SSL_is_init_finished(tls_io_instance->ssl))
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_OK);
        result = 0;
    }
    else
    {
        SSL_do_handshake(tls_io_instance->ssl);
        if (SSL_is_init_finished(tls_io_instance->ssl))
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
            indicate_open_complete(tls_io_instance, IO_OPEN_OK);
            result = 0;
        }
        else
        {
            if (write_outgoing_bytes(tls_io_instance, NULL, NULL) != 0)
            {
                result = __LINE__;
                LogError("Error in write_outgoing_bytes.\r\n");
            }
            else
            {
                if (SSL_is_init_finished(tls_io_instance->ssl))
                {
                    tls_io_instance->tlsio_state = TLSIO_STATE_OPEN;
                    indicate_open_complete(tls_io_instance, IO_OPEN_OK);
                }

                result = 0;
            }
        }
    }

    return result;
}

static void on_underlying_io_close_complete(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    switch (tls_io_instance->tlsio_state)
    {
    default:
    case TLSIO_STATE_NOT_OPEN:
    case TLSIO_STATE_OPEN:
        break;

    case TLSIO_STATE_OPENING_UNDERLYING_IO:
    case TLSIO_STATE_IN_HANDSHAKE:
        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        break;

    case TLSIO_STATE_CLOSING:
        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;

        if (tls_io_instance->on_io_close_complete == NULL)
        {
            LogError("NULL on_io_close_complete.\r\n");
        }
        else
        {
            tls_io_instance->on_io_close_complete(tls_io_instance->on_io_close_complete_context);
        }
        break;
    }
}

static void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    if (tls_io_instance->tlsio_state == TLSIO_STATE_OPENING_UNDERLYING_IO)
    {
        if (open_result == IO_OPEN_OK)
        {
            if (SSL_is_init_finished(tls_io_instance->ssl))
            {
                if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
                {
                    indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
                    LogError("Error in xio_close.\r\n");
                }
            }
            else
            {
                tls_io_instance->tlsio_state = TLSIO_STATE_IN_HANDSHAKE;

                if (send_handshake_bytes(tls_io_instance) != 0)
                {
                    if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
                    {
                        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
                        LogError("Error in xio_close.\r\n");
                    }
                }
            }
        }
        else
        {
            indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPENING_UNDERLYING_IO.\r\n");
        }
    }
}

static void on_underlying_io_error(void* context)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    switch (tls_io_instance->tlsio_state)
    {
    default:
        break;

    case TLSIO_STATE_OPENING_UNDERLYING_IO:
    case TLSIO_STATE_IN_HANDSHAKE:
        tls_io_instance->tlsio_state = TLSIO_STATE_NOT_OPEN;
        indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
        break;

    case TLSIO_STATE_OPEN:
        indicate_error(tls_io_instance);
        break;
    }
}

static int decode_ssl_received_bytes(TLS_IO_INSTANCE* tls_io_instance)
{
    int result = 0;
    unsigned char buffer[64];

    int rcv_bytes = 1;
    while (rcv_bytes > 0)
    {
        rcv_bytes = SSL_read(tls_io_instance->ssl, buffer, sizeof(buffer));
        if (rcv_bytes > 0)
        {
            if (tls_io_instance->on_bytes_received == NULL)
            {
                LogError("NULL on_bytes_received.\r\n");
            }
            else
            {
                tls_io_instance->on_bytes_received(tls_io_instance->on_bytes_received_context, buffer, rcv_bytes);
            }
        }
    }

    return result;
}

static void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
{
    TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)context;

    int written = BIO_write(tls_io_instance->in_bio, buffer, size);
    if (written != size)
    {
        tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
        indicate_error(tls_io_instance);
        LogError("Error in BIO_write.\r\n");
    }
    else
    {
        switch (tls_io_instance->tlsio_state)
        {
        default:
            break;

        case TLSIO_STATE_IN_HANDSHAKE:
            if (send_handshake_bytes(tls_io_instance) != 0)
            {
                if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
                {
                    indicate_open_complete(tls_io_instance, IO_OPEN_ERROR);
                    LogError("Error in xio_close.\r\n");
                }
            }
            break;

        case TLSIO_STATE_OPEN:
            if (decode_ssl_received_bytes(tls_io_instance) != 0)
            {
                tls_io_instance->tlsio_state = TLSIO_STATE_ERROR;
                indicate_error(tls_io_instance);
                LogError("Error in decode_ssl_received_bytes.\r\n");
            }
            break;
        }
    }
}

int tlsio_openssl_init(void)
{
    (void)SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    return 0;
}

void tlsio_openssl_deinit(void)
{
}

CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters, LOGGER_LOG logger_log)
{
    TLSIO_CONFIG* tls_io_config = io_create_parameters;
    TLS_IO_INSTANCE* result;

    if (tls_io_config == NULL)
    {
        result = NULL;
        LogError("NULL tls_io_config.\r\n");
    }
    else
    {
        result = malloc(sizeof(TLS_IO_INSTANCE));
        if (result == NULL)
        {
            LogError("Failed allocating TLSIO instance.\r\n");
        }
        else
        {
            SOCKETIO_CONFIG socketio_config;

            socketio_config.hostname = tls_io_config->hostname;
            socketio_config.port = tls_io_config->port;
            socketio_config.accepted_socket = NULL;

            result->on_bytes_received = NULL;
            result->on_bytes_received_context = NULL;

            result->on_io_open_complete = NULL;
            result->on_io_open_complete_context = NULL;

            result->on_io_close_complete = NULL;
            result->on_io_close_complete_context = NULL;

            result->on_io_error = NULL;
            result->on_io_error_context = NULL;

            result->logger_log = logger_log;
            result->tlsio_state = TLSIO_STATE_NOT_OPEN;

            result->ssl_context = SSL_CTX_new(TLSv1_method());
            if (result->ssl_context == NULL)
            {
                free(result);
                result = NULL;
                LogError("Failed allocating OpenSSL context.\r\n");
            }
            else
            {
                result->in_bio = BIO_new(BIO_s_mem());
                if (result->in_bio == NULL)
                {
                    SSL_CTX_free(result->ssl_context);
                    free(result);
                    result = NULL;
                    LogError("Failed BIO_new for in BIO.\r\n");
                }
                else
                {
                    result->out_bio = BIO_new(BIO_s_mem());
                    if (result->out_bio == NULL)
                    {
                        (void)BIO_free(result->in_bio);
                        SSL_CTX_free(result->ssl_context);
                        free(result);
                        result = NULL;
                        LogError("Failed BIO_new for out BIO.\r\n");
                    }
                    else
                    {
                        const IO_INTERFACE_DESCRIPTION* underlying_io_interface = socketio_get_interface_description();
                        if (underlying_io_interface == NULL)
                        {
                            (void)BIO_free(result->in_bio);
                            (void)BIO_free(result->out_bio);
                            SSL_CTX_free(result->ssl_context);
                            free(result);
                            result = NULL;
                            LogError("Failed getting socket IO interface description.\r\n");
                        }
                        else
                        {
                            result->underlying_io = xio_create(underlying_io_interface, &socketio_config, logger_log);
                            if ((result->underlying_io == NULL) ||
                                (BIO_set_mem_eof_return(result->in_bio, -1) <= 0) ||
                                (BIO_set_mem_eof_return(result->out_bio, -1) <= 0))
                            {
                                (void)BIO_free(result->in_bio);
                                (void)BIO_free(result->out_bio);
                                SSL_CTX_free(result->ssl_context);
                                free(result);
                                result = NULL;
                                LogError("Failed xio_create.\r\n");
                            }
                            else
                            {
                                SSL_CTX_set_verify(result->ssl_context, SSL_VERIFY_PEER, NULL);

                                /* Specifies that the default locations for which CA certificates are loaded should be used. */
                                if (SSL_CTX_set_default_verify_paths(result->ssl_context) != 1)
                                {
                                    /* This is only a warning to the user. They can still specify the certificate via SetOption. */
                                    LogInfo("WARNING: Unable to specify the default location for CA certificates on this platform.\r\n");
                                }

                                result->ssl = SSL_new(result->ssl_context);
                                if (result->ssl == NULL)
                                {
                                    (void)BIO_free(result->in_bio);
                                    (void)BIO_free(result->out_bio);
                                    SSL_CTX_free(result->ssl_context);
                                    free(result);
                                    result = NULL;
                                    LogError("Failed creating OpenSSL instance.\r\n");
                                }
                                else
                                {
                                    SSL_set_bio(result->ssl, result->in_bio, result->out_bio);
                                    SSL_set_connect_state(result->ssl);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.\r\n");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;
        SSL_free(tls_io_instance->ssl);
        SSL_CTX_free(tls_io_instance->ssl_context);

        xio_destroy(tls_io_instance->underlying_io);
        free(tls_io);
    }
}

int tlsio_openssl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.\r\n");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN.\r\n");
        }
        else
        {
            tls_io_instance->on_io_open_complete = on_io_open_complete;
            tls_io_instance->on_io_open_complete_context = on_io_open_complete_context;

            tls_io_instance->on_bytes_received = on_bytes_received;
            tls_io_instance->on_bytes_received_context = on_bytes_received_context;

            tls_io_instance->on_io_error = on_io_error;
            tls_io_instance->on_io_error_context = on_io_error_context;

            tls_io_instance->tlsio_state = TLSIO_STATE_OPENING_UNDERLYING_IO;

            if (xio_open(tls_io_instance->underlying_io, on_underlying_io_open_complete, tls_io_instance, on_underlying_io_bytes_received, tls_io_instance, on_underlying_io_error, tls_io_instance) != 0)
            {
                result = __LINE__;
                LogError("Error in xio_open.\r\n");
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.\r\n");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state == TLSIO_STATE_NOT_OPEN) ||
            (tls_io_instance->tlsio_state == TLSIO_STATE_CLOSING))
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_NOT_OPEN or TLSIO_STATE_CLOSING.\r\n");
        }
        else
        {
            tls_io_instance->tlsio_state = TLSIO_STATE_CLOSING;
            tls_io_instance->on_io_close_complete = on_io_close_complete;
            tls_io_instance->on_io_close_complete_context = callback_context;

            if (xio_close(tls_io_instance->underlying_io, on_underlying_io_close_complete, tls_io_instance) != 0)
            {
                result = __LINE__;
                LogError("Error in xio_close.\r\n");
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int tlsio_openssl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if (tls_io == NULL)
    {
        result = __LINE__;
        LogError("NULL tls_io.\r\n");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (tls_io_instance->tlsio_state != TLSIO_STATE_OPEN)
        {
            result = __LINE__;
            LogError("Invalid tlsio_state. Expected state is TLSIO_STATE_OPEN.\r\n");
        }
        else
        {
            int res = SSL_write(tls_io_instance->ssl, buffer, size);
            if (res != size)
            {
                result = __LINE__;
                LogError("SSL_write error.\r\n");
            }
            else
            {
                if (write_outgoing_bytes(tls_io_instance, on_send_complete, callback_context) != 0)
                {
                    result = __LINE__;
                    LogError("Error in write_outgoing_bytes.\r\n");
                }
                else
                {
                    result = 0;
                }
            }
        }
    }

    return result;
}

void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io)
{
    if (tls_io == NULL)
    {
        LogError("NULL tls_io.\r\n");
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if ((tls_io_instance->tlsio_state != TLSIO_STATE_NOT_OPEN) &&
            (tls_io_instance->tlsio_state != TLSIO_STATE_ERROR))
        {
            xio_dowork(tls_io_instance->underlying_io);
        }
    }
}

int tlsio_openssl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
{
    int result;
    X509_STORE* cert_store;
    BIO* cert_memory_bio;
    X509* xcert;

    if (tls_io == NULL || optionName == NULL)
    {
        result = __LINE__;
    }
    else
    {
        TLS_IO_INSTANCE* tls_io_instance = (TLS_IO_INSTANCE*)tls_io;

        if (strcmp("TrustedCerts", optionName) == 0)
        {
            cert_store = SSL_CTX_get_cert_store(tls_io_instance->ssl_context);
            if (cert_store == NULL)
            {
                result = __LINE__;
            }
            else
            {
                cert_memory_bio = BIO_new_mem_buf((void*)value, -1);

                if (cert_memory_bio == NULL)
                {
                    result = __LINE__;
                }
                else
                {
                    xcert = PEM_read_bio_X509(cert_memory_bio, NULL, NULL, NULL);

                    if (xcert == NULL) {
                        result = __LINE__;
                    }
                    else
                    {
                        if (X509_STORE_add_cert(cert_store, xcert) != 1)
                        {
                            X509_free(xcert);
                            result = __LINE__;
                        }
                        else
                        {
                            result = 0;
                        }
                    }
                    BIO_free(cert_memory_bio);
                }
            }
        }
        else
        {
            result = xio_setoption(tls_io_instance->underlying_io, optionName, value);
        }
    }

    return result;
}

const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void)
{
    return &tlsio_openssl_interface_description;
}
