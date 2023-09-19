#include "config/private.h"

Config::Wrap::Wrap(Config *_conf)
{
    conf = _conf;
}

Config * Config::Wrap::operator->() {
    return conf;
}

Config::Wrap::operator Config*(){return conf;}

std::vector<Config>::iterator Config::Wrap::begin() {return conf->begin();}
std::vector<Config>::iterator Config::Wrap::end() {return conf->end();}

Config::ConstWrap::ConstWrap(const Config *_conf)
{
    conf = _conf;
}

const Config * Config::ConstWrap::operator->() const {
    return conf;
}

Config::ConstWrap::operator const Config*() const {return conf;}
