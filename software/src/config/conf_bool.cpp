#include "config/private.h"

bool *Config::ConfBool::getVal() { return &value; }
const bool *Config::ConfBool::getVal() const { return &value; }
