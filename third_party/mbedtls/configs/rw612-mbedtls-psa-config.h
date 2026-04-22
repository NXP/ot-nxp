/*
 *  Copyright (c) 2025, The OpenThread Authors.
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

#ifndef RW612_MBEDTLS_PSA_USER_CONFIG_H
#define RW612_MBEDTLS_PSA_USER_CONFIG_H

/* MBEDTLS_PSA_CRYPTO_C required MBEDTLS_CTR_DRBG_C or MBEDTLS_HMAC_DRBG_C or MBEDTLS_ENTROPY_C or MBEDTLS_PSA_CRYPTO_EXTERNAL_RNG */
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_NO_PLATFORM_ENTROPY

// Algorithm acceleration flags
#define MBEDTLS_PSA_ACCEL_ALG_SHA_256 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDSA 1
/* Couldn't enable it due to MBEDTLS_HMAC_DRBG_C definition */
//#define MBEDTLS_PSA_ACCEL_ALG_DETERMINISTIC_ECDSA 1
#define MBEDTLS_PSA_ACCEL_ALG_ECDH 1
/* couldn't set it because openthread init need to do HMAC with SHA256 and it is not supported with HW acceleration

openthread caller: psa_mac_sign_setup(operation, aKey->mKeyRef, PSA_ALG_HMAC(PSA_ALG_SHA_256))
mcuxClPsaDriver_psa_driver_wrapper_mac_setupLayer function return error:

    No support for multipart Hmac
    if(PSA_ALG_IS_HMAC(alg) == true)
    {
        return PSA_ERROR_NOT_SUPPORTED;
    }
*/
//#define MBEDTLS_PSA_ACCEL_ALG_HMAC 1

// Need Software implementation when using PSA_ALG_HKDF(PSA_ALG_SHA_256) like in PsaKdf::Init
//#define MBEDTLS_PSA_ACCEL_ALG_HKDF 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXTRACT 1
#define MBEDTLS_PSA_ACCEL_ALG_HKDF_EXPAND 1
#define MBEDTLS_PSA_ACCEL_ALG_CCM 1

// Not supported: els_pkc_transparent_import_key return PSA_ERROR_NOT_SUPPORTED
//#define MBEDTLS_PSA_ACCEL_ALG_ECB_NO_PADDING 1

#define MBEDTLS_PSA_ACCEL_ALG_PBKDF2_AES_CMAC_PRF_128 1

// Curve acceleration flags
#define MBEDTLS_PSA_ACCEL_ECC_SECP_R1_256 1

// Key type acceleration flags
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_AES 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_HMAC 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_DERIVE 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_RAW_DATA 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_PASSWORD 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_BASIC 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_IMPORT 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_EXPORT 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_GENERATE 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_PUBLIC_KEY 1
#define MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1


/* Config added by config_adjust_legacy_from_psa.h:

Due to non accelerated MBEDTLS_PSA_ACCEL_ALG_DETERMINISTIC_ECDSA :
#define MBEDTLS_PSA_ECC_ACCEL_INCOMPLETE_ALGS

Due to MBEDTLS_PSA_ECC_ACCEL_INCOMPLETE_ALGS:

#define MBEDTLS_PSA_BUILTIN_ECC_SECP_R1_256 1
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_PUBLIC_KEY 1
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_KEY_PAIR_BASIC 1

Special case: we don't support cooked key derivation in drivers yet :
#undef MBEDTLS_PSA_ACCEL_KEY_TYPE_ECC_KEY_PAIR_DERIVE

Due to previous undef:
#define MBEDTLS_PSA_ECC_ACCEL_INCOMPLETE_KEY_TYPES
#define MBEDTLS_PSA_BUILTIN_KEY_TYPE_ECC_KEY_PAIR_DERIVE 1
#define MBEDTLS_ECP_LIGHT
#define MBEDTLS_BIGNUM_C

Due to non accelerated MBEDTLS_PSA_BUILTIN_ALG_HMAC

#define MBEDTLS_PSA_BUILTIN_ALG_HMAC 1

*/

/* Optimization for elliptic curves operations*/
#define MBEDTLS_ECP_NIST_OPTIM

#endif /* RW612_MBEDTLS_PSA_USER_CONFIG_H */