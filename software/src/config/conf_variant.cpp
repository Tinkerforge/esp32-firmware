#include "config/private.h"

Config::ConfVariant::Val::Val() : e(Empty{}) {}
Config::ConfVariant::Val::~Val() {}

Config::ConfVariant::ConfVariant(ConfString s) : tag(Tag::STRING), updated(0xFF), val() {new(&val.s)  ConfString{s};}
Config::ConfVariant::ConfVariant(ConfFloat f)  : tag(Tag::FLOAT),  updated(0xFF), val() {new(&val.f)  ConfFloat{f};}
Config::ConfVariant::ConfVariant(ConfInt i)    : tag(Tag::INT),    updated(0xFF), val() {new(&val.i)  ConfInt{i};}
Config::ConfVariant::ConfVariant(ConfUint u)   : tag(Tag::UINT),   updated(0xFF), val() {new(&val.u)  ConfUint{u};}
Config::ConfVariant::ConfVariant(ConfBool b)   : tag(Tag::BOOL),   updated(0xFF), val() {new(&val.b)  ConfBool{b};}
Config::ConfVariant::ConfVariant(ConfArray a)  : tag(Tag::ARRAY),  updated(0xFF), val() {new(&val.a)  ConfArray{a};}
Config::ConfVariant::ConfVariant(ConfObject o) : tag(Tag::OBJECT), updated(0xFF), val() {new(&val.o)  ConfObject{o};}
Config::ConfVariant::ConfVariant(ConfUnion un) : tag(Tag::UNION),  updated(0xFF), val() {new(&val.un) ConfUnion{un};}

Config::ConfVariant::ConfVariant() : tag(Tag::EMPTY), updated(0xFF), val() {}

Config::ConfVariant::ConfVariant(const ConfVariant &cpy)
{
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
    }
#ifdef __GNUC__
    __builtin_unreachable();
#endif
}
