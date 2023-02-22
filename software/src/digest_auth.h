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

#pragma once

#include <Arduino.h>

#define DEFAULT_REALM "esp32-lib"

typedef struct AuthFields {
    String username;
    String realm;
    String nonce;
    String uri;
    String response;
    String qop;
    String nc;
    String cnonce;

    String opaque;

    bool success;
} AuthFields;

AuthFields parseDigestAuth(const char *header);

String requestDigestAuthentication(const char * realm);
bool checkDigestAuthentication(const AuthFields &fields, const char * method, const char * username, const char * password, const char * realm, bool passwordIsHash, const char * nonce, const char * opaque, const char * uri);

//for storing hashed versions on the device that can be authenticated against
String generateDigestHash(const char * username, const char * password, const char * realm);
