#include "spine_connection.h"

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

extern EEBus eebus;

SpineConnection::SpineConnection()
{
}

void SpineConnection::process_datagram(String datagram)
{
    logger.printfln("SPINE Process datagram: %s", datagram.c_str());
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};

    DeserializationError error = deserializeJson(doc, datagram, DeserializationOption::NestingLimit(SPINE_CONNECTION_MAX_DEPTH));

    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
        return;
    }

    JsonVariant datagram_header = doc["datagram"][0]["header"];
    JsonVariant datagram_payload = doc["datagram"][1]["payload"];
    if (datagram_header.isNull() || datagram_payload.isNull()) {
        logger.printfln("Error: No datagram header or payload found");
        return;
    }
    logger.printfln("SPINE Process header: %s", datagram_header.as<String>().c_str());
    logger.printfln("SPINE Process datagram: %s", datagram_payload.as<String>().c_str());
}
