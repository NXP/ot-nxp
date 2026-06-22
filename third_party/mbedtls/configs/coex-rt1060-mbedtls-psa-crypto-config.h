/*
 *  Copyright (c) 2026, NXP.
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

#ifndef COEX_RT1060_MBEDTLS_PSA_CRYPTO_CONFIG_H
#define COEX_RT1060_MBEDTLS_PSA_CRYPTO_CONFIG_H

#include "mcux_mbedtls_psa_crypto_config.h"

#if defined(CONFIG_WPA_SUPP_MBEDTLS) && (CONFIG_WPA_SUPP_MBEDTLS == 1)
#undef MBEDTLS_SSL_MAX_CONTENT_LEN
#undef PSA_WANT_ALG_JPAKE
/* wpa_supplicant mbedtls extend config */
#include "wpa_supp_els_pkc_mbedtls_config.h"
#endif /* CONFIG_WPA_SUPP_MBEDTLS */

#ifndef PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC
#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC 1
#endif

#undef MBEDTLS_PSA_ACCEL_ECC_SECP_R1_192
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_R1_224
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_R1_256
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_R1_384
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_R1_521
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_K1_192
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_K1_224
#undef MBEDTLS_PSA_ACCEL_ECC_SECP_K1_256
#undef MBEDTLS_PSA_ACCEL_ECC_MONTGOMERY_255
#undef MBEDTLS_PSA_ACCEL_ECC_MONTGOMERY_448
#undef MBEDTLS_PSA_ACCEL_ECC_BRAINPOOL_P_R1_256
#undef MBEDTLS_PSA_ACCEL_ECC_BRAINPOOL_P_R1_384
#undef MBEDTLS_PSA_ACCEL_ECC_BRAINPOOL_P_R1_512

#endif /* COEX_RT1060_MBEDTLS_PSA_CRYPTO_CONFIG_H */
