#include "cool_string.h"

void CoolString::setLength(int len)
{
    setLen(len);
}

unsigned int CoolString::getCapacity()
{
    return capacity();
}

void CoolString::shrinkToFit()
{
    changeBuffer(len());
}

void CoolString::make_invalid() {
    this->invalidate();
}

/* Currently broken for SSO strings
char* CoolString::releaseOwnership(size_t *len) {
    char *p = const_cast<char *>(c_str());
    *len = length();

    // String::init is marked inline and cannot be called
    // from here. copy the body of String::init to here
    setSSO(false);
    setBuffer(nullptr);
    setCapacity(0);
    setLen(0);

    return p;
}
*/
