#pragma once

#include "WString.h"

// Arduino String that allows accessing more methods.
class CoolString : public String
{
public:
    using String::String;
    using String::operator=;

    void setLength(int len);
    unsigned int getCapacity();
    void shrinkToFit();
    void make_invalid();
    //char *releaseOwnership(size_t *len);
};
