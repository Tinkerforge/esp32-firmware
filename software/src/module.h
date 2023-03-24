#pragma once

// FIXME Replace this file with whatever has not been committed yet.

class IModule
{
public:
    void pre_setup(){};
    void setup(){};
    void register_urls(){};
    void loop(){};

    bool initialized = false;
};
