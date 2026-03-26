/*
 *  Copyright (c) 2026, The OpenThread Authors.
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

/**
 * @file
 *   This file implements OT ECDSA operations
 *
 */

#include <string.h>

#include <crypto/ecdsa.hpp>
#include <crypto/sha256.hpp>
#include <openthread/error.h>
#include <openthread/platform/crypto.h>

#include "common/code_utils.hpp"
#include "common/debug.hpp"

#include "psa/crypto.h"
#include <mbedtls/asn1.h>
#include "mcux_psa_s2xx_key_locations.h"

using namespace ot;
using namespace Crypto;

#if OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_PSA

extern "C" void otPlatPsaInit()
{
    /*
     * linker extracts object files from them only when needed to resolve undefined symbols
     * need to call this function from otSysInit, otherwise the strong symbols are not used
     */
}

static Error PsaToOtError(psa_status_t aStatus)
{
    Error error;

    switch (aStatus)
    {
    case PSA_SUCCESS:
        error = kErrorNone;
        break;
    case PSA_ERROR_INVALID_ARGUMENT:
        error = kErrorInvalidArgs;
        break;
    case PSA_ERROR_BUFFER_TOO_SMALL:
        error = kErrorNoBufs;
        break;
    default:
        error = kErrorFailed;
        break;
    }

    return error;
}

static psa_key_type_t ToPsaKeyType(otCryptoKeyType aType)
{
    psa_key_type_t type;

    switch (aType)
    {
    case OT_CRYPTO_KEY_TYPE_RAW:
        type = PSA_KEY_TYPE_RAW_DATA;
        break;
    case OT_CRYPTO_KEY_TYPE_AES:
        type = PSA_KEY_TYPE_AES;
        break;
    case OT_CRYPTO_KEY_TYPE_HMAC:
        type = PSA_KEY_TYPE_HMAC;
        break;
    case OT_CRYPTO_KEY_TYPE_ECDSA:
        type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
        break;
    case OT_CRYPTO_KEY_TYPE_DERIVE:
        type = PSA_KEY_TYPE_DERIVE;
        break;
    default:
        type = PSA_KEY_TYPE_NONE;
        break;
    }

    return type;
}

static psa_algorithm_t ToPsaAlgorithm(otCryptoKeyAlgorithm aAlgorithm)
{
    psa_algorithm_t algorithm;

    switch (aAlgorithm)
    {
    case OT_CRYPTO_KEY_ALG_AES_ECB:
        algorithm = PSA_ALG_ECB_NO_PADDING;
        break;
    case OT_CRYPTO_KEY_ALG_HMAC_SHA_256:
        algorithm = PSA_ALG_HMAC(PSA_ALG_SHA_256);
        break;
    case OT_CRYPTO_KEY_ALG_ECDSA:
        algorithm = PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256);
        break;
    case OT_CRYPTO_KEY_ALG_HKDF_SHA256:
        algorithm = PSA_ALG_HKDF(PSA_ALG_SHA_256);
        break;
    default:
        algorithm = PSA_ALG_NONE;
        break;
    }

    return algorithm;
}

static psa_key_usage_t ToPsaKeyUsage(int aUsage)
{
    psa_key_usage_t usage = 0;

    if (aUsage & OT_CRYPTO_KEY_USAGE_EXPORT)
    {
        usage |= PSA_KEY_USAGE_EXPORT;
    }

    if (aUsage & OT_CRYPTO_KEY_USAGE_ENCRYPT)
    {
        usage |= PSA_KEY_USAGE_ENCRYPT;
    }

    if (aUsage & OT_CRYPTO_KEY_USAGE_DECRYPT)
    {
        usage |= PSA_KEY_USAGE_DECRYPT;
    }

    if (aUsage & OT_CRYPTO_KEY_USAGE_SIGN_HASH)
    {
        usage |= PSA_KEY_USAGE_SIGN_HASH;
    }

    if (aUsage & OT_CRYPTO_KEY_USAGE_VERIFY_HASH)
    {
        usage |= PSA_KEY_USAGE_VERIFY_HASH;
    }

    if (aUsage & OT_CRYPTO_KEY_USAGE_DERIVE)
    {
        usage |= PSA_KEY_USAGE_DERIVE;
    }

    return usage;
}

static Error ValidateKeyUsage(int aUsage)
{
    Error error = kErrorNone;

    // Check if only supported flags have been passed
    static constexpr int supportedFlags = OT_CRYPTO_KEY_USAGE_EXPORT | OT_CRYPTO_KEY_USAGE_ENCRYPT |
                                          OT_CRYPTO_KEY_USAGE_DECRYPT | OT_CRYPTO_KEY_USAGE_SIGN_HASH |
                                          OT_CRYPTO_KEY_USAGE_VERIFY_HASH | OT_CRYPTO_KEY_USAGE_DERIVE;

    VerifyOrExit((aUsage & ~supportedFlags) == 0, error = kErrorInvalidArgs);

exit:
    return error;
}

static Error ExtractPrivateKeyInfo(const uint8_t *aAsn1KeyPair,
                                   size_t         aAsn1KeyPairLen,
                                   size_t        *aKeyOffset,
                                   size_t        *aKeyLen)
{
    int                  ret;
    Error                error = kErrorNone;
    unsigned char       *p     = const_cast<unsigned char *>(aAsn1KeyPair);
    const unsigned char *end   = p + aAsn1KeyPairLen;
    size_t               len;

    // Parse the ASN.1 SEQUENCE headers
    ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(ret == 0, error = kErrorInvalidArgs);

    // Parse the version (INTEGER)
    ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_INTEGER);
    VerifyOrExit(ret == 0, error = kErrorInvalidArgs);

    // Skip the version.
    p += len;

    // Parse the private key (OCTET STRING)
    ret = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
    VerifyOrExit(ret == 0, error = kErrorInvalidArgs);

    // Check if the private key includes a padding byte (0x00)
    if (*p == 0x00)
    {
        p++;
        len--; // Skip the padding byte and reduce length by 1
    }

    *aKeyOffset = (p - aAsn1KeyPair);
    *aKeyLen    = len;

exit:
    return error;
}

otError otPlatCryptoImportKey(otCryptoKeyRef      *aKeyRef,
                              otCryptoKeyType      aKeyType,
                              otCryptoKeyAlgorithm aKeyAlgorithm,
                              int                  aKeyUsage,
                              otCryptoKeyStorage   aKeyPersistence,
                              const uint8_t       *aKey,
                              size_t               aKeyLen)
{
    Error                error      = kErrorNone;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    SuccessOrExit(error = ValidateKeyUsage(aKeyUsage));
    VerifyOrExit(aKeyRef != nullptr && aKey != nullptr, error = kErrorInvalidArgs);

    // PSA Crypto API expects the private key to be provided, not the full ASN1 buffer.
    if (aKeyType == OT_CRYPTO_KEY_TYPE_ECDSA)
    {
        size_t pkOffset;
        size_t pkLength;

        SuccessOrExit(error = ExtractPrivateKeyInfo(aKey, aKeyLen, &pkOffset, &pkLength));

        // Overwrite the content of the key.
        aKey += pkOffset;
        aKeyLen = pkLength;

        psa_set_key_bits(&attributes, 256);
    }

    psa_set_key_type(&attributes, ToPsaKeyType(aKeyType));
    psa_set_key_algorithm(&attributes, ToPsaAlgorithm(aKeyAlgorithm));
    psa_set_key_usage_flags(&attributes, ToPsaKeyUsage(aKeyUsage));

    switch (aKeyPersistence)
    {
    case OT_CRYPTO_KEY_STORAGE_PERSISTENT:
        psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_PERSISTENT);
        psa_set_key_id(&attributes, *aKeyRef);
        break;
    case OT_CRYPTO_KEY_STORAGE_VOLATILE:
        psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);
        break;
    default:
        OT_ASSERT(false);
    }

    error = PsaToOtError(psa_import_key(&attributes, aKey, aKeyLen, aKeyRef));

exit:
    psa_reset_key_attributes(&attributes);

    return error;
}

#if OPENTHREAD_CONFIG_ECDSA_ENABLE

otError otPlatCryptoEcdsaGenerateAndImportKey(otCryptoKeyRef aKeyRef)
{
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_status_t         status;
    psa_key_id_t         keyId = static_cast<psa_key_id_t>(aKeyRef);

    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
                    PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_LOCATION_S200_KEY_STORAGE_NON_EL2GO));
    psa_set_key_id(&attributes, keyId);
    psa_set_key_bits(&attributes, 256);

    status = psa_generate_key(&attributes, &keyId);
    VerifyOrExit(status == PSA_SUCCESS);

exit:
    psa_reset_key_attributes(&attributes);

    return PsaToOtError(status);
}

otError otPlatCryptoEcdsaSignUsingKeyRef(otCryptoKeyRef               aKeyRef,
                                        const otPlatCryptoSha256Hash *aHash,
                                        otPlatCryptoEcdsaSignature   *aSignature)
{
    psa_status_t status;
    size_t       signatureLen;

    status = psa_sign_hash(aKeyRef, PSA_ALG_ECDSA(PSA_ALG_SHA_256), aHash->m8, OT_CRYPTO_SHA256_HASH_SIZE,
                           aSignature->m8, OT_CRYPTO_ECDSA_SIGNATURE_SIZE, &signatureLen);
    VerifyOrExit(status == PSA_SUCCESS);

    OT_ASSERT(signatureLen == OT_CRYPTO_ECDSA_SIGNATURE_SIZE);

exit:
    return PsaToOtError(status);
}

otError otPlatCryptoEcdsaVerifyUsingKeyRef(otCryptoKeyRef                    aKeyRef,
                                           const otPlatCryptoSha256Hash     *aHash,
                                           const otPlatCryptoEcdsaSignature *aSignature)
{
    psa_status_t status;

    status = psa_verify_hash(aKeyRef, PSA_ALG_ECDSA(PSA_ALG_SHA_256), aHash->m8,
                             OT_CRYPTO_SHA256_HASH_SIZE, aSignature->m8, OT_CRYPTO_ECDSA_SIGNATURE_SIZE);
    VerifyOrExit(status == PSA_SUCCESS);

exit:
    return PsaToOtError(status);
}

#endif // OPENTHREAD_CONFIG_ECDSA_ENABLE
#endif // OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_PSA