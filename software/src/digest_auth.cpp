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

#include "mbedtls/md5.h"
#include "event_log.h"

extern EventLog logger;

static bool getMD5(uint8_t * data, uint16_t len, char * output){//33 bytes or more
    mbedtls_md5_context _ctx;
  uint8_t i;
  uint8_t * _buf = (uint8_t*)malloc(16);
  if(_buf == NULL)
    return false;
  memset(_buf, 0x00, 16);

  mbedtls_md5_init(&_ctx);
  mbedtls_md5_starts_ret(&_ctx);
  mbedtls_md5_update_ret(&_ctx, data, len);
  mbedtls_md5_finish_ret(&_ctx, _buf);
  for(i = 0; i < 16; i++) {
    sprintf(output + (i * 2), "%02x", _buf[i]);
  }
  free(_buf);
  return true;
}

static String genRandomString(){
  uint8_t data[16] = {0};
  uint32_t t = micros();
  uint8_t i = 0;
  memcpy(data, &t, sizeof(t));
#ifdef _BUILD_TIME_
  t = _BUILD_TIME_;
  memcpy(data + 4, &t, sizeof(t));
#else
  t = millis();
  memcpy(data + 4, &t, sizeof(t));
#endif
  uint32_t r = rand();
  memcpy(data + 8, &r, sizeof(r));
  r = rand();
  memcpy(data + 12, &r, sizeof(r));

  char buf[33] = {0};
  getMD5(data, 16, buf);

  return String(buf);
}

static String genRandomMD5(){
  uint32_t r = rand();
  char * out = (char*)malloc(33);
  if(out == NULL || !getMD5((uint8_t*)(&r), 4, out))
    return "";
  String res = String(out);
  free(out);
  return res;
}

static String stringMD5(const String& in){
  char * out = (char*)malloc(33);
  if(out == NULL || !getMD5((uint8_t*)(in.c_str()), in.length(), out))
    return "";
  String res = String(out);
  free(out);
  return res;
}

String requestDigestAuthentication(const char * realm){
  String header = "realm=\"";
  if(realm == NULL)
    header.concat("esp32-lib");
  else
    header.concat(realm);
  header.concat( "\", qop=\"auth\", nonce=\"");
  header.concat(genRandomString());
  header.concat("\", opaque=\"");
  header.concat(genRandomString());
  header.concat("\"");
  return header;
}

bool checkDigestAuthentication(const char * header, const char * method, const char * username, const char * password, const char * realm, bool passwordIsHash, const char * nonce, const char * opaque, const char * uri){
  if(username == NULL || password == NULL || header == NULL || method == NULL){
    logger.printfln("AUTH FAIL: missing requred fields");
    return false;
  }

  String myHeader = String(header);
  int nextBreak = myHeader.indexOf(",");
  if(nextBreak < 0){
    logger.printfln("AUTH FAIL: no variables");
    return false;
  }

  String myUsername = String();
  String myRealm = String();
  String myNonce = String();
  String myUri = String();
  String myResponse = String();
  String myQop = String();
  String myNc = String();
  String myCnonce = String();

  myHeader += ", ";
  do {
    String avLine = myHeader.substring(0, nextBreak);
    avLine.trim();
    myHeader = myHeader.substring(nextBreak+1);
    nextBreak = myHeader.indexOf(",");

    int eqSign = avLine.indexOf("=");
    if(eqSign < 0){
      logger.printfln("AUTH FAIL: no = sign");
      return false;
    }
    String varName = avLine.substring(0, eqSign);
    avLine = avLine.substring(eqSign + 1);
    if(avLine.startsWith("\"")){
      avLine = avLine.substring(1, avLine.length() - 1);
    }

    if(varName.equals("username")){
      if(!avLine.equals(username)){
        logger.printfln("AUTH FAIL: username");
        return false;
      }
      myUsername = avLine;
    } else if(varName.equals("realm")){
      if(realm != NULL && !avLine.equals(realm)){
        logger.printfln("AUTH FAIL: realm");
        return false;
      }
      myRealm = avLine;
    } else if(varName.equals("nonce")){
      if(nonce != NULL && !avLine.equals(nonce)){
        logger.printfln("AUTH FAIL: nonce");
        return false;
      }
      myNonce = avLine;
    } else if(varName.equals("opaque")){
      if(opaque != NULL && !avLine.equals(opaque)){
        logger.printfln("AUTH FAIL: opaque");
        return false;
      }
    } else if(varName.equals("uri")){
      if(uri != NULL && !avLine.equals(uri)){
        logger.printfln("AUTH FAIL: uri");
        return false;
      }
      myUri = avLine;
    } else if(varName.equals("response")){
      myResponse = avLine;
    } else if(varName.equals("qop")){
      myQop = avLine;
    } else if(varName.equals("nc")){
      myNc = avLine;
    } else if(varName.equals("cnonce")){
      myCnonce = avLine;
    }
  } while(nextBreak > 0);

  String ha1 = (passwordIsHash) ? String(password) : stringMD5(myUsername + ":" + myRealm + ":" + String(password));
  String ha2 = String(method) + ":" + myUri;
  String response = ha1 + ":" + myNonce + ":" + myNc + ":" + myCnonce + ":" + myQop + ":" + stringMD5(ha2);

  if(myResponse.equals(stringMD5(response))){
    logger.printfln("AUTH SUCCESS");
    return true;
  }

  logger.printfln("AUTH FAIL: password");
  return false;
}
