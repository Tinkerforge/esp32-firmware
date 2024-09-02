/*
    This is a stripped down version supporting only digest authentication.
    Taken from https://github.com/me-no-dev/ESPAsyncWebServer
*/
/*
  Asynchronous WebServer library for Espressif MCUs

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "digest_auth.h"

#include <mbedtls/md5.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "build.h"
#include "cool_string.h"

#define MD5_LEN 32

static bool getMD5(uint8_t * data, uint16_t len, char * output){//33 bytes or more
    mbedtls_md5_context _ctx;
    uint8_t i;
    uint8_t _buf[16];
    memset(_buf, 0x00, 16);

    mbedtls_md5_init(&_ctx);
    mbedtls_md5_starts_ret(&_ctx);
    mbedtls_md5_update_ret(&_ctx, data, len);
    mbedtls_md5_finish_ret(&_ctx, _buf);
    for(i = 0; i < 16; i++) {
        sprintf(output + (i * 2), "%02x", _buf[i]);
    }
    return true;
}

static String genRandomString(){
    uint8_t data[16] = {0};
    uint32_t t = micros();
    memcpy(data, &t, sizeof(t));
    t = build_timestamp();
    memcpy(data + 4, &t, sizeof(t));
    uint32_t r = esp_random();
    memcpy(data + 8, &r, sizeof(r));
    r = esp_random();
    memcpy(data + 12, &r, sizeof(r));

    char buf[33] = {0};
    getMD5(data, 16, buf);

    return String(buf);
}

static String stringMD5(const String& in){
    CoolString out;
    if (!out.reserve(MD5_LEN + 1))
        return "";

    if(!getMD5((uint8_t*)(in.c_str()), in.length(), out.begin()))
        return "";
    out.setLength(MD5_LEN);
    return out;
}

String requestDigestAuthentication(const char * realm){
    String header = "realm=\"";
    if(realm == NULL) {
        header.concat(DEFAULT_REALM);
    } else {
        header.concat(realm);
    }
    header.concat( "\", qop=\"auth\", nonce=\"");
    header.concat(genRandomString());
    header.concat("\", opaque=\"");
    header.concat(genRandomString());
    header.concat("\"");
    return header;
}

AuthFields parseDigestAuth(const char *header)
{
    AuthFields result;
    result.success = false;

    if (header == nullptr) {
        logger.printfln("AUTH FAIL: missing required fields");
        return result;
    }

    String myHeader = String(header);
    int nextBreak = myHeader.indexOf(",");
    if (nextBreak < 0) {
        logger.printfln("AUTH FAIL: no variables");
        return result;
    }

    myHeader += ", ";
    do {
        String avLine = myHeader.substring(0, nextBreak);
        avLine.trim();
        myHeader = myHeader.substring(nextBreak + 1);
        nextBreak = myHeader.indexOf(",");

        int eqSign = avLine.indexOf("=");
        if (eqSign < 0) {
            logger.printfln("AUTH FAIL: no = sign");
            return result;
        }

        String varName = avLine.substring(0, eqSign);
        avLine = avLine.substring(eqSign + 1);
        if (avLine.startsWith("\"")) {
            avLine = avLine.substring(1, avLine.length() - 1);
        }

        if (varName.equals("username")) {
            result.username = avLine;
        } else if (varName.equals("realm")) {
            result.realm = avLine;
        } else if (varName.equals("nonce")) {
            result.nonce = avLine;
        } else if (varName.equals("opaque")) {
            result.opaque = avLine;
        } else if (varName.equals("uri")) {
            result.uri = avLine;
        } else if (varName.equals("response")) {
            result.response = avLine;
        } else if (varName.equals("qop")) {
            result.qop = avLine;
        } else if (varName.equals("nc")) {
            result.nc = avLine;
        } else if (varName.equals("cnonce")) {
            result.cnonce = avLine;
        }
    } while (nextBreak > 0);

    result.success = true;
    return result;
}

bool checkDigestAuthentication(const AuthFields &fields, const char * method, const char * username, const char * password, const char * realm, bool passwordIsHash, const char * nonce, const char * opaque, const char * uri){
    if (username == NULL || password == NULL || method == NULL) {
        logger.printfln("AUTH FAIL: missing required fields");
        return false;
    }

    if (!fields.username.equals(username)) {
        logger.printfln("AUTH FAIL: username");
        return false;
    }

    if (realm != NULL && !fields.realm.equals(realm)) {
        logger.printfln("AUTH FAIL: realm");
        return false;
    } else if (realm == NULL && !fields.realm.equals(DEFAULT_REALM) && !fields.realm.equals("asyncesp")) {
        logger.printfln("AUTH FAIL: realm");
        return false;
    }

    if (nonce != NULL && !fields.nonce.equals(nonce)) {
        logger.printfln("AUTH FAIL: nonce");
        return false;
    }

    if (opaque != NULL && !fields.opaque.equals(opaque)) {
        logger.printfln("AUTH FAIL: opaque");
        return false;
    }
    if (uri != NULL && !fields.uri.equals(uri)) {
        logger.printfln("AUTH FAIL: uri");
        return false;
    }

    String ha1 = (passwordIsHash) ? String(password) : stringMD5(fields.username + ":" + fields.realm + ":" + String(password));
    if (passwordIsHash && ha1.length() < MD5_LEN) {
        logger.printfln("AUTH FAIL: out of memory");
        return false;
    }

    String ha2 = String(method) + ":" + fields.uri;
    ha2 = stringMD5(ha2);
    if (ha2.length() < MD5_LEN) {
        logger.printfln("AUTH FAIL: out of memory");
        return false;
    }

    String response = ha1 + ":" + fields.nonce + ":" + fields.nc + ":" + fields.cnonce + ":" + fields.qop + ":" + ha2;

    if (fields.response.equals(stringMD5(response))) {
        return true;
    }

    logger.printfln("AUTH FAIL: password");
    return false;
}

String generateDigestHash(const char * username, const char * password, const char * realm){
    if(username == NULL || password == NULL || realm == NULL){
        return "";
    }

    return stringMD5(String(username) + ':' + realm + ':' + password);
}
