/*
 *  Copyright (c) 2021-2026, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OT_NXP_MBEDTLS_CONFIG_H
#define OT_NXP_MBEDTLS_CONFIG_H

#include "get_mbedtls_version.h"
#include "mcux_mbedtls_accelerator_config.h"
#include "openthread-core-config.h"
#include <openthread/config.h>
#include <openthread/platform/logging.h>
#include <openthread/platform/crypto.h>

#include <stdio.h>

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf
#define MBEDTLS_PLATFORM_C

#ifndef MBEDTLS_PSA_CRYPTO_STORAGE_C
#define MBEDTLS_PSA_CRYPTO_STORAGE_C 1
#endif /* MBEDTLS_PSA_CRYPTO_STORAGE_C */

// Algorithm flags
#define PSA_WANT_ALG_SHA_256 1
#define PSA_WANT_ALG_ECDSA 1
#define PSA_WANT_ALG_DETERMINISTIC_ECDSA 1
#define PSA_WANT_ALG_ECDH 1
#define PSA_WANT_ALG_HMAC 1
#define PSA_WANT_ALG_HKDF 1
#define PSA_WANT_ALG_HKDF_EXTRACT 1
#define PSA_WANT_ALG_HKDF_EXPAND 1
#define PSA_WANT_ALG_CCM 1
#define PSA_WANT_ALG_GCM 1
#define PSA_WANT_ALG_TLS12_ECJPAKE_TO_PMS 1
#define PSA_WANT_ALG_TLS12_PRF 1
/* Openthread and factory data support */
#define PSA_WANT_ALG_ECB_NO_PADDING 1
#define PSA_WANT_ALG_JPAKE
/* Use HMAC instead as CMAC execute software aes operation */
//#define PSA_WANT_ALG_CMAC 1

#define PSA_WANT_ALG_PBKDF2_AES_CMAC_PRF_128 1
#define PSA_WANT_KEY_TYPE_PASSWORD 1
#define PSA_WANT_KEY_TYPE_RAW_DATA 1

// Curve flags
#define PSA_WANT_ECC_SECP_R1_256 1

// Key type flags
#define PSA_WANT_KEY_TYPE_AES 1
#define PSA_WANT_KEY_TYPE_HMAC 1
#define PSA_WANT_KEY_TYPE_DERIVE 1

#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_IMPORT 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_EXPORT 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1
#define PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY 1

#if USE_RTOS
#ifndef MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_C
#endif
#ifndef MBEDTLS_THREADING_ALT
#define MBEDTLS_THREADING_ALT
#endif
#endif

#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PSA_CRYPTO_CONFIG
#define MBEDTLS_USE_PSA_CRYPTO

#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_PK_WRITE_C

#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH

/*
 * Define MBEDTLS_SSL_MAX_CONTENT_LEN as 2000 if OPENTHREAD_CONFIG_BLE_TCAT_ENABLE
 * Define MBEDTLS_SSL_MAX_CONTENT_LEN as 900 if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
*/
#ifdef MBEDTLS_SSL_MAX_CONTENT_LEN
#undef MBEDTLS_SSL_MAX_CONTENT_LEN
#endif /* MBEDTLS_SSL_MAX_CONTENT_LEN */
#define MBEDTLS_SSL_MAX_CONTENT_LEN      2000 /**< Maxium fragment length in bytes */

#define MBEDTLS_SSL_IN_CONTENT_LEN       MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_OUT_CONTENT_LEN      MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_CIPHERSUITES         MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8

#define MBEDTLS_SSL_DTLS_CONNECTION_ID_COMPAT 0
#define MBEDTLS_SSL_DTLS_CONNECTION_ID 0

#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_ECP_C

#endif // OT_NXP_MBEDTLS_CONFIG_H
