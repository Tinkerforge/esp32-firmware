// Encode an ISO 15118 xmldsig SignedInfo fragment with libcbv2g.
// EcoG's encoder generates different (also schema-valid) EXI for the
// iso20 fragment. Verified against a production CPS: its -20 signatures
// verify over the libcbv2g encoding and not over EcoG's, so libcbv2g
// matches the ecosystem. For the -2 SignedInfo both encoders produce
// identical bytes.
//
// Usage: pnc_exi_helper <2|20> <digest_hex>
// Prints the SignedInfo EXI as hex.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbv2g/common/exi_bitstream.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Datatypes.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Encoder.h"
#include "cbv2g/iso_2/iso2_msgDefDatatypes.h"
#include "cbv2g/iso_2/iso2_msgDefEncoder.h"

static const char *CANON = "http://www.w3.org/TR/canonical-exi/";

static size_t from_hex(const char *hex, uint8_t *out, size_t out_cap)
{
    size_t n = strlen(hex) / 2;
    if (n > out_cap) n = out_cap;
    for (size_t i = 0; i < n; i++) sscanf(hex + i * 2, "%2hhx", &out[i]);
    return n;
}

static void print_hex(const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) printf("%02x", buf[i]);
    printf("\n");
}

int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "usage: %s <2|20> <digest_hex>\n", argv[0]); return 2; }
    int mode = atoi(argv[1]);
    uint8_t digest[64];
    size_t digest_len = from_hex(argv[2], digest, sizeof(digest));
    const char *sig_alg = digest_len == 64 ? "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha512"
                                           : "http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha256";
    const char *dig_alg = digest_len == 64 ? "http://www.w3.org/2001/04/xmlenc#sha512"
                                           : "http://www.w3.org/2001/04/xmlenc#sha256";

    uint8_t out[2048];
    exi_bitstream_t stream;
    size_t pos = 0;
    exi_bitstream_init(&stream, out, sizeof(out), 0, NULL);
    (void)pos;

    int rc;
    if (mode == 20) {
        struct iso20_xmldsigFragment frag;
        init_iso20_xmldsigFragment(&frag);
        frag.SignedInfo_isUsed = 1;
        struct iso20_SignedInfoType *si = &frag.SignedInfo;
        si->Id_isUsed = 0;
        strcpy(si->CanonicalizationMethod.Algorithm.characters, CANON);
        si->CanonicalizationMethod.Algorithm.charactersLen = strlen(CANON);
        si->CanonicalizationMethod.ANY_isUsed = 0;
        strcpy(si->SignatureMethod.Algorithm.characters, sig_alg);
        si->SignatureMethod.Algorithm.charactersLen = strlen(sig_alg);
        si->SignatureMethod.HMACOutputLength_isUsed = 0;
        si->SignatureMethod.ANY_isUsed = 0;
        si->Reference.arrayLen = 1;
        struct iso20_ReferenceType *ref = &si->Reference.array[0];
        ref->Id_isUsed = 0;
        ref->Type_isUsed = 0;
        ref->URI_isUsed = 1;
        strcpy(ref->URI.characters, "#id1");
        ref->URI.charactersLen = 4;
        ref->Transforms_isUsed = 1;
        strcpy(ref->Transforms.Transform.Algorithm.characters, CANON);
        ref->Transforms.Transform.Algorithm.charactersLen = strlen(CANON);
        ref->Transforms.Transform.ANY_isUsed = 0;
        ref->Transforms.Transform.XPath_isUsed = 0;
        strcpy(ref->DigestMethod.Algorithm.characters, dig_alg);
        ref->DigestMethod.Algorithm.charactersLen = strlen(dig_alg);
        ref->DigestMethod.ANY_isUsed = 0;
        memcpy(ref->DigestValue.bytes, digest, digest_len);
        ref->DigestValue.bytesLen = digest_len;
        rc = encode_iso20_xmldsigFragment(&stream, &frag);
    } else {
        struct iso2_xmldsigFragment frag;
        init_iso2_xmldsigFragment(&frag);
        frag.SignedInfo_isUsed = 1;
        struct iso2_SignedInfoType *si = &frag.SignedInfo;
        si->Id_isUsed = 0;
        strcpy(si->CanonicalizationMethod.Algorithm.characters, CANON);
        si->CanonicalizationMethod.Algorithm.charactersLen = strlen(CANON);
        si->CanonicalizationMethod.ANY_isUsed = 0;
        strcpy(si->SignatureMethod.Algorithm.characters, sig_alg);
        si->SignatureMethod.Algorithm.charactersLen = strlen(sig_alg);
        si->SignatureMethod.HMACOutputLength_isUsed = 0;
        si->SignatureMethod.ANY_isUsed = 0;
        si->Reference.arrayLen = 1;
        struct iso2_ReferenceType *ref = &si->Reference.array[0];
        ref->Id_isUsed = 0;
        ref->Type_isUsed = 0;
        ref->URI_isUsed = 1;
        strcpy(ref->URI.characters, "#id1");
        ref->URI.charactersLen = 4;
        ref->Transforms_isUsed = 1;
        strcpy(ref->Transforms.Transform.Algorithm.characters, CANON);
        ref->Transforms.Transform.Algorithm.charactersLen = strlen(CANON);
        ref->Transforms.Transform.ANY_isUsed = 0;
        ref->Transforms.Transform.XPath_isUsed = 0;
        strcpy(ref->DigestMethod.Algorithm.characters, dig_alg);
        ref->DigestMethod.Algorithm.charactersLen = strlen(dig_alg);
        ref->DigestMethod.ANY_isUsed = 0;
        memcpy(ref->DigestValue.bytes, digest, digest_len);
        ref->DigestValue.bytesLen = digest_len;
        rc = encode_iso2_xmldsigFragment(&stream, &frag);
    }

    if (rc != 0) { fprintf(stderr, "encode failed: %d\n", rc); return 1; }
    print_hex(out, exi_bitstream_get_length(&stream));
    return 0;
}
