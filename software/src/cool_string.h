#pragma once

#include "WString.h"

// Arduino String that allows accessing more methods.
class CoolString : public String
{
public:
    using String::String;

    void setLength(int len);
    unsigned int getCapacity();
    void shrinkToFit();
    //char *releaseOwnership(size_t *len);
};
