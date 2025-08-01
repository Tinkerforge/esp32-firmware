/* esp32-firmware
 * Copyright (C) 2020-2024 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config/private.h"

Config::ConfVariant::Val::Val() : e(Empty{}) {}
Config::ConfVariant::Val::~Val() {}

Config::ConfVariant::ConfVariant(ConfString s) : tag(Tag::STRING), updated(0xFF), val() {new(&val.s)   ConfString{s};}
Config::ConfVariant::ConfVariant(ConfFloat f)  : tag(Tag::FLOAT),  updated(0xFF), val() {new(&val.f)   ConfFloat{f};}
Config::ConfVariant::ConfVariant(ConfInt i)    : tag(Tag::INT),    updated(0xFF), val() {new(&val.i)   ConfInt{i};}
Config::ConfVariant::ConfVariant(ConfUint u)   : tag(Tag::UINT),   updated(0xFF), val() {new(&val.u)   ConfUint{u};}
Config::ConfVariant::ConfVariant(ConfBool b)   : tag(Tag::BOOL),   updated(0xFF), val() {new(&val.b)   ConfBool{b};}
Config::ConfVariant::ConfVariant(ConfArray a)  : tag(Tag::ARRAY),  updated(0xFF), val() {new(&val.a)   ConfArray{a};}
Config::ConfVariant::ConfVariant(ConfObject o) : tag(Tag::OBJECT), updated(0xFF), val() {new(&val.o)   ConfObject{o};}
Config::ConfVariant::ConfVariant(ConfUnion un) : tag(Tag::UNION),  updated(0xFF), val() {new(&val.un)  ConfUnion{un};}
Config::ConfVariant::ConfVariant(ConfInt52 i)  : tag(Tag::INT64),  updated(0xFF), val() {new(&val.i64) ConfInt52{i};}
Config::ConfVariant::ConfVariant(ConfUint53 u) : tag(Tag::UINT64), updated(0xFF), val() {new(&val.u64) ConfUint53{u};}
Config::ConfVariant::ConfVariant(ConfUint16 u) : tag(Tag::UINT16), updated(0xFF), val() {new(&val.u16) ConfUint16{u};}
Config::ConfVariant::ConfVariant(ConfInt16 u)  : tag(Tag::INT16),  updated(0xFF), val() {new(&val.i16) ConfInt16{u};}
Config::ConfVariant::ConfVariant(ConfUint8 u)  : tag(Tag::UINT8),  updated(0xFF), val() {new(&val.u8)  ConfUint8{u};}
Config::ConfVariant::ConfVariant(ConfInt8 u)   : tag(Tag::INT8),   updated(0xFF), val() {new(&val.i8)  ConfInt8{u};}

Config::ConfVariant::ConfVariant() : tag(Tag::EMPTY), updated(0xFF), val() {}

Config::ConfVariant::ConfVariant(const ConfVariant &cpy)
{
    switch (cpy.tag) {
        case ConfVariant::Tag::EMPTY:
            new(&val.e) Empty(cpy.val.e);
            break;
        case ConfVariant::Tag::STRING:
            new(&val.s) ConfString(cpy.val.s);
            break;
        case ConfVariant::Tag::FLOAT:
            new(&val.f) ConfFloat(cpy.val.f);
            break;
        case ConfVariant::Tag::INT:
            new(&val.i) ConfInt(cpy.val.i);
            break;
        case ConfVariant::Tag::UINT:
            new(&val.u) ConfUint(cpy.val.u);
            break;
        case ConfVariant::Tag::BOOL:
            new(&val.b) ConfBool(cpy.val.b);
            break;
        case ConfVariant::Tag::ARRAY:
            new(&val.a) ConfArray(cpy.val.a);
            break;
        case ConfVariant::Tag::OBJECT:
            new(&val.o) ConfObject(cpy.val.o);
            break;
        case ConfVariant::Tag::UNION:
            new(&val.un) ConfUnion(cpy.val.un);
            break;
        case ConfVariant::Tag::INT64:
            new(&val.i64) ConfInt52(cpy.val.i64);
            break;
        case ConfVariant::Tag::UINT64:
            new(&val.u64) ConfUint53(cpy.val.u64);
            break;
        case ConfVariant::Tag::UINT16:
            new(&val.u16) ConfUint16(cpy.val.u16);
            break;
        case ConfVariant::Tag::INT16:
            new(&val.i16) ConfInt16(cpy.val.i16);
            break;
        case ConfVariant::Tag::UINT8:
            new(&val.u8) ConfUint8(cpy.val.u8);
            break;
        case ConfVariant::Tag::INT8:
            new(&val.i8) ConfInt8(cpy.val.i8);
            break;
    }
    this->tag = cpy.tag;
    this->updated = cpy.updated;
}

Config::ConfVariant &Config::ConfVariant::operator=(const ConfVariant &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    if (tag != Tag::EMPTY)
        destroyUnionMember();

    switch (cpy.tag) {
        case ConfVariant::Tag::EMPTY:
            new(&val.e) Empty(cpy.val.e);
            break;
        case ConfVariant::Tag::STRING:
            new(&val.s) ConfString(cpy.val.s);
            break;
        case ConfVariant::Tag::FLOAT:
            new(&val.f) ConfFloat(cpy.val.f);
            break;
        case ConfVariant::Tag::INT:
            new(&val.i) ConfInt(cpy.val.i);
            break;
        case ConfVariant::Tag::UINT:
            new(&val.u) ConfUint(cpy.val.u);
            break;
        case ConfVariant::Tag::BOOL:
            new(&val.b) ConfBool(cpy.val.b);
            break;
        case ConfVariant::Tag::ARRAY:
            new(&val.a) ConfArray(cpy.val.a);
            break;
        case ConfVariant::Tag::OBJECT:
            new(&val.o) ConfObject(cpy.val.o);
            break;
        case ConfVariant::Tag::UNION:
            new(&val.un) ConfUnion(cpy.val.un);
            break;
        case ConfVariant::Tag::INT64:
            new(&val.i64) ConfInt52(cpy.val.i64);
            break;
        case ConfVariant::Tag::UINT64:
            new(&val.u64) ConfUint53(cpy.val.u64);
            break;
        case ConfVariant::Tag::UINT16:
            new(&val.u16) ConfUint16(cpy.val.u16);
            break;
        case ConfVariant::Tag::INT16:
            new(&val.i16) ConfInt16(cpy.val.i16);
            break;
        case ConfVariant::Tag::UINT8:
            new(&val.u8) ConfUint8(cpy.val.u8);
            break;
        case ConfVariant::Tag::INT8:
            new(&val.i8) ConfInt8(cpy.val.i8);
            break;
    }
    this->tag = cpy.tag;
    this->updated = cpy.updated;

    return *this;
}

void Config::ConfVariant::destroyUnionMember()
{
    switch (tag) {
        case ConfVariant::Tag::EMPTY:
            val.e.~Empty();
            break;
        case ConfVariant::Tag::STRING:
            val.s.~ConfString();
            break;
        case ConfVariant::Tag::FLOAT:
            val.f.~ConfFloat();
            break;
        case ConfVariant::Tag::INT:
            val.i.~ConfInt();
            break;
        case ConfVariant::Tag::UINT:
            val.u.~ConfUint();
            break;
        case ConfVariant::Tag::BOOL:
            val.b.~ConfBool();
            break;
        case ConfVariant::Tag::ARRAY:
            val.a.~ConfArray();
            break;
        case ConfVariant::Tag::OBJECT:
            val.o.~ConfObject();
            break;
        case ConfVariant::Tag::UNION:
            val.un.~ConfUnion();
            break;
        case ConfVariant::Tag::INT64:
            val.i64.~ConfInt52();
            break;
        case ConfVariant::Tag::UINT64:
            val.u64.~ConfUint53();
            break;
        case ConfVariant::Tag::UINT16:
            val.u16.~ConfUint16();
            break;
        case ConfVariant::Tag::INT16:
            val.i16.~ConfInt16();
            break;
        case ConfVariant::Tag::UINT8:
            val.u8.~ConfUint8();
            break;
        case ConfVariant::Tag::INT8:
            val.i8.~ConfInt8();
            break;
    }
}

Config::ConfVariant::~ConfVariant()
{
    destroyUnionMember();
}

const char *Config::ConfVariant::getVariantName() const
{
    switch (tag) {
        case ConfVariant::Tag::EMPTY:
            return "Empty";
        case ConfVariant::Tag::STRING:
            return val.s.variantName;
        case ConfVariant::Tag::FLOAT:
            return val.f.variantName;
        case ConfVariant::Tag::INT:
            return val.i.variantName;
        case ConfVariant::Tag::UINT:
            return val.u.variantName;
        case ConfVariant::Tag::BOOL:
            return val.b.variantName;
        case ConfVariant::Tag::ARRAY:
            return val.a.variantName;
        case ConfVariant::Tag::OBJECT:
            return val.o.variantName;
        case ConfVariant::Tag::UNION:
            return val.un.variantName;
        case ConfVariant::Tag::INT64:
            return val.i64.variantName;
        case ConfVariant::Tag::UINT64:
            return val.u64.variantName;
        case ConfVariant::Tag::UINT16:
            return val.u16.variantName;
        case ConfVariant::Tag::INT16:
            return val.i16.variantName;
        case ConfVariant::Tag::UINT8:
            return val.u8.variantName;
        case ConfVariant::Tag::INT8:
            return val.i8.variantName;
    }
    esp_system_abort("getVariantName: ConfVariant has unknown type!");
}

Config::ConfVariant::ConfVariant(ConfVariant &&cpy) {
    switch (cpy.tag) {
        case ConfVariant::Tag::EMPTY:
            new(&val.e) Empty(std::move(cpy.val.e));
            break;
        case ConfVariant::Tag::STRING:
            new(&val.s) ConfString(std::move(cpy.val.s));
            break;
        case ConfVariant::Tag::FLOAT:
            new(&val.f) ConfFloat(std::move(cpy.val.f));
            break;
        case ConfVariant::Tag::INT:
            new(&val.i) ConfInt(std::move(cpy.val.i));
            break;
        case ConfVariant::Tag::UINT:
            new(&val.u) ConfUint(std::move(cpy.val.u));
            break;
        case ConfVariant::Tag::BOOL:
            new(&val.b) ConfBool(std::move(cpy.val.b));
            break;
        case ConfVariant::Tag::ARRAY:
            new(&val.a) ConfArray(std::move(cpy.val.a));
            break;
        case ConfVariant::Tag::OBJECT:
            new(&val.o) ConfObject(std::move(cpy.val.o));
            break;
        case ConfVariant::Tag::UNION:
            new(&val.un) ConfUnion(std::move(cpy.val.un));
            break;
        case ConfVariant::Tag::INT64:
            new(&val.i64) ConfInt52(std::move(cpy.val.i64));
            break;
        case ConfVariant::Tag::UINT64:
            new(&val.u64) ConfUint53(std::move(cpy.val.u64));
            break;
        case ConfVariant::Tag::UINT16:
            new(&val.u16) ConfUint16(std::move(cpy.val.u16));
            break;
        case ConfVariant::Tag::INT16:
            new(&val.i16) ConfInt16(std::move(cpy.val.i16));
            break;
        case ConfVariant::Tag::UINT8:
            new(&val.u8) ConfUint8(std::move(cpy.val.u8));
            break;
        case ConfVariant::Tag::INT8:
            new(&val.i8) ConfInt8(std::move(cpy.val.i8));
            break;
    }
    this->tag = cpy.tag;
    this->updated = cpy.updated;

    cpy.tag = ConfVariant::Tag::EMPTY;
}

Config::ConfVariant &Config::ConfVariant::operator=(ConfVariant &&cpy) {
    if (tag != Tag::EMPTY)
        destroyUnionMember();

    switch (cpy.tag) {
        case ConfVariant::Tag::EMPTY:
            new(&val.e) Empty(std::move(cpy.val.e));
            break;
        case ConfVariant::Tag::STRING:
            new(&val.s) ConfString(std::move(cpy.val.s));
            break;
        case ConfVariant::Tag::FLOAT:
            new(&val.f) ConfFloat(std::move(cpy.val.f));
            break;
        case ConfVariant::Tag::INT:
            new(&val.i) ConfInt(std::move(cpy.val.i));
            break;
        case ConfVariant::Tag::UINT:
            new(&val.u) ConfUint(std::move(cpy.val.u));
            break;
        case ConfVariant::Tag::BOOL:
            new(&val.b) ConfBool(std::move(cpy.val.b));
            break;
        case ConfVariant::Tag::ARRAY:
            new(&val.a) ConfArray(std::move(cpy.val.a));
            break;
        case ConfVariant::Tag::OBJECT:
            new(&val.o) ConfObject(std::move(cpy.val.o));
            break;
        case ConfVariant::Tag::UNION:
            new(&val.un) ConfUnion(std::move(cpy.val.un));
            break;
        case ConfVariant::Tag::INT64:
            new(&val.i64) ConfInt52(std::move(cpy.val.i64));
            break;
        case ConfVariant::Tag::UINT64:
            new(&val.u64) ConfUint53(std::move(cpy.val.u64));
            break;
        case ConfVariant::Tag::UINT16:
            new(&val.u16) ConfUint16(std::move(cpy.val.u16));
            break;
        case ConfVariant::Tag::INT16:
            new(&val.i16) ConfInt16(std::move(cpy.val.i16));
            break;
        case ConfVariant::Tag::UINT8:
            new(&val.u8) ConfUint8(std::move(cpy.val.u8));
            break;
        case ConfVariant::Tag::INT8:
            new(&val.i8) ConfInt8(std::move(cpy.val.i8));
            break;
    }
    this->tag = cpy.tag;
    this->updated = cpy.updated;

    cpy.tag = ConfVariant::Tag::EMPTY;

    return *this;
}
