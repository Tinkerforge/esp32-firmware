#pragma once

#include <WString.h>

// Arduino String that allows accessing more methods.
class CoolString : public String
{
public:
    using String::String;
    using String::operator=;

    void setLength(int len);
    unsigned int getCapacity() const;
    void shrinkToFit();
    inline void make_invalid() {this->invalidate();};
    inline bool is_sso() const {return this->isSSO();};
    //char *releaseOwnership(size_t *len);
};

static_assert(sizeof(String) == sizeof(CoolString), "It is only safe to import operator= from the parent class if the child class doesn't add any member variables that must be assigned.");
