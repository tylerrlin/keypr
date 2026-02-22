/*
 * This file is part of the Pico FIDO distribution (https://github.com/polhenarejos/pico-fido).
 * Copyright (c) 2022 Pol Henarejos.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pico_keys.h"
#include "ctap2_cbor.h"
#include "hid/ctap_hid.h"
#include "fido.h"
#include "ctap.h"
#include "files.h"
#include "apdu.h"
#include "version.h"

int cbor_get_info()
{
    CborEncoder encoder, mapEncoder, arrayEncoder, mapEncoder2;
    CborError error = CborNoError;
    cbor_encoder_init(&encoder, ctap_resp->init.data + 1, CTAP_MAX_CBOR_PAYLOAD, 0);
    // FIDO_2_1 + makeCredUvNotRqd: advertise as FIDO 2.1 so browsers recognise
    // this as a passkey-capable device, but with makeCredUvNotRqd=true so they
    // don't require UV for credential creation. clientPin is ABSENT (not false)
    // so Chrome won't attempt PIN setup. pinUvAuthProtocols MUST be present for
    // Chrome to treat this as a real CTAP2 device (without it Chrome falls back
    // to U2F-only mode and skips the device entirely for passkeys).
    // Fields: versions(1) + extensions(2) + aaguid(3) + options(4) + maxMsgSize(5)
    //       + pinUvAuthProtocols(6) + maxCredCount(7) + maxCredIdLen(8) + transports(9)
    //       + algorithms(0xA) + maxLargeBlobArray(0xB) + forcePINChange(0xC)
    //       + minPINLength(0xD) + firmwareVersion(0xE)
    // NOTE: maxCredBlobLen(0xF) is OMITTED — Chrome requires it iff "credBlob"
    // is in the extensions array. We don't advertise "credBlob" so sending 0xF
    // causes ReadCTAPGetInfoResponse to return nullopt → U2F-only fallback.
    // transports(0x09) = ["usb"] tells the browser this is a USB key so it
    // shows the "Use your security key" dialog on webauthn.io and other sites.
    // vendorCmds(0x15) is OMITTED — libfido2 and Chrome don't understand it and
    // it causes fido_dev_get_cbor_info to return FIDO_ERR_RX, breaking getInfo.
    // = 14 base fields only
    uint8_t lfields = 14;
    file_t *ef_pin_policy = search_by_fid(EF_PIN_COMPLEXITY_POLICY, NULL, SPECIFY_EF);
    if (file_has_data(ef_pin_policy))
    {
        lfields += 2;
    }
    CBOR_CHECK(cbor_encoder_create_map(&encoder, &mapEncoder, lfields));

    // 0x01 versions — FIDO_2_1 required for browsers to treat device as passkey-capable
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x01));
    CBOR_CHECK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, 3));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "U2F_V2"));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "FIDO_2_0"));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "FIDO_2_1"));
    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));

    // 0x02 extensions
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x02));
    CBOR_CHECK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, 2));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "hmac-secret"));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "credProtect"));
    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x03));
    CBOR_CHECK(cbor_encode_byte_string(&mapEncoder, aaguid, sizeof(aaguid)));

    // 0x04 options:
    //   rk=true: resident key / discoverable credential support (required for passkeys)
    //   up=true: user presence supported
    //   uv=true: built-in user verification (button press = UV; required for Chrome to
    //     accept passkey assertion without a PIN — Chrome forces UV for discoverable
    //     credential assertions on external authenticators; advertising uv=true makes
    //     Chrome use kNoTokenInternalUV path and send uv=true in the CTAP request)
    //   makeCredUvNotRqd=true: UV not required to create a credential (FIDO 2.1 §6.1.3)
    //   clientPin ABSENT (not present, not false): device does not support clientPin
    //     → Chrome will NOT attempt PIN setup or send authenticatorSelection
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x04));
    CBOR_CHECK(cbor_encoder_create_map(&mapEncoder, &arrayEncoder, 4));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "rk"));
    CBOR_CHECK(cbor_encode_boolean(&arrayEncoder, true));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "up"));
    CBOR_CHECK(cbor_encode_boolean(&arrayEncoder, true));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "uv"));
    CBOR_CHECK(cbor_encode_boolean(&arrayEncoder, true));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "makeCredUvNotRqd"));
    CBOR_CHECK(cbor_encode_boolean(&arrayEncoder, true));
    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x05));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, MAX_MSG_SIZE));

    // 0x06 pinUvAuthProtocols — MUST be present for Chrome to treat device as
    // CTAP2 (not U2F-only). clientPin is absent from options so no PIN setup.
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x06));
    CBOR_CHECK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, 2));
    CBOR_CHECK(cbor_encode_uint(&arrayEncoder, 1));
    CBOR_CHECK(cbor_encode_uint(&arrayEncoder, 2));
    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x07));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, MAX_CREDENTIAL_COUNT_IN_LIST)); // MAX_CRED_COUNT_IN_LIST

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x08));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, MAX_CRED_ID_LENGTH)); // MAX_CRED_ID_MAX_LENGTH

    // 0x09 transports — tell the browser this is a USB authenticator so it
    // shows the security key UI (e.g. webauthn.io "Use your security key")
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x09));
    CBOR_CHECK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, 1));
    CBOR_CHECK(cbor_encode_text_stringz(&arrayEncoder, "usb"));
    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x0A));

    uint8_t curves = 3;
#ifdef MBEDTLS_EDDSA_C
    curves++;
#endif
#ifndef ENABLE_EMULATION
    if (phy_data.enabled_curves & PHY_CURVE_SECP256K1)
    {
#endif
        curves++;
#ifndef ENABLE_EMULATION
    }
#endif
    CBOR_CHECK(cbor_encoder_create_array(&mapEncoder, &arrayEncoder, curves));
    CBOR_CHECK(COSE_public_key(FIDO2_ALG_ES256, &arrayEncoder, &mapEncoder2));
#ifdef MBEDTLS_EDDSA_C
    CBOR_CHECK(COSE_public_key(FIDO2_ALG_EDDSA, &arrayEncoder, &mapEncoder2));
#endif
    CBOR_CHECK(COSE_public_key(FIDO2_ALG_ES384, &arrayEncoder, &mapEncoder2));
    CBOR_CHECK(COSE_public_key(FIDO2_ALG_ES512, &arrayEncoder, &mapEncoder2));
#ifndef ENABLE_EMULATION
    if (phy_data.enabled_curves & PHY_CURVE_SECP256K1)
    {
#endif
        CBOR_CHECK(COSE_public_key(FIDO2_ALG_ES256K, &arrayEncoder, &mapEncoder2));
#ifndef ENABLE_EMULATION
    }
#endif

    CBOR_CHECK(cbor_encoder_close_container(&mapEncoder, &arrayEncoder));

    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x0B));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, MAX_LARGE_BLOB_SIZE)); // maxSerializedLargeBlobArray

    file_t *ef_minpin = search_by_fid(EF_MINPINLEN, NULL, SPECIFY_EF);
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x0C));
    if (file_has_data(ef_minpin) && file_get_data(ef_minpin)[1] == 1)
    {
        CBOR_CHECK(cbor_encode_boolean(&mapEncoder, true));
    }
    else
    {
        CBOR_CHECK(cbor_encode_boolean(&mapEncoder, false));
    }
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x0D));
    if (file_has_data(ef_minpin))
    {
        CBOR_CHECK(cbor_encode_uint(&mapEncoder, *file_get_data(ef_minpin))); // minPINLength
    }
    else
    {
        CBOR_CHECK(cbor_encode_uint(&mapEncoder, 4)); // minPINLength
    }
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x0E));
    CBOR_CHECK(cbor_encode_uint(&mapEncoder, PICO_FIDO_VERSION)); // firmwareVersion

#ifndef ENABLE_EMULATION
    if (file_has_data(ef_pin_policy))
    {
        CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x1B));
        CBOR_CHECK(cbor_encode_boolean(&mapEncoder, true));
        CBOR_CHECK(cbor_encode_uint(&mapEncoder, 0x1C));
        CBOR_CHECK(cbor_encode_byte_string(&mapEncoder, file_get_data(ef_pin_policy) + 2, file_get_size(ef_pin_policy) - 2));
    }
#endif
    CBOR_CHECK(cbor_encoder_close_container(&encoder, &mapEncoder));
err:
    if (error != CborNoError)
    {
        return -CTAP2_ERR_INVALID_CBOR;
    }
    res_APDU_size = (uint16_t)cbor_encoder_get_buffer_size(&encoder, ctap_resp->init.data + 1);
    return 0;
}
