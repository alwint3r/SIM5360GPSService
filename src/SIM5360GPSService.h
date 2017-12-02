#ifndef COMPONENT_SIM_GPS_SERVICE_H
#define COMPONENT_SIM_GPS_SERVICE_H

#include <Stream.h>
#include <Arduino.h>
#include <esp_log.h>

#undef max
#undef min

class SIM5360GPSData {
public:
    SIM5360GPSData();
    ~SIM5360GPSData();

    void parse(char* data, int length);

    int64_t getLatitude() {
        return latitude;
    }

    int64_t getLongitude() {
        return longitude;
    }

    int32_t getAltitude() {
        return altitude;
    }

    int32_t getSpeed() {
        return speed;
    }
private:
    int readUntil(char* buffer, int length, char token, int start = 0);
    int getStringSlice(char* haystack, char* dest, int start, int end, char skip = ' ');

    int64_t latitude;
    int64_t longitude;
    int32_t altitude; // meters
    int32_t speed; // knot
};

class SIM5360GPSService {
public:
    SIM5360GPSService();
    ~SIM5360GPSService();
    bool begin(Stream* serial);
    void run();

    SIM5360GPSData& getGpsData() {
        return gpsData;
    }
private:
    String cleanupResponse(char* data, const char* tobeStriped = "");
    int getSerialResponse(char* buffer, uint32_t timeout);
    int sendCommandAndWaitForResponse(const char* command, uint32_t timeout = 3000);

    const char* TAG = "SIMGPS";

    Stream* serial;
    SIM5360GPSData gpsData;
    char* responseBuffer = nullptr;
    size_t responseBufferSize = 1024;
};

#endif