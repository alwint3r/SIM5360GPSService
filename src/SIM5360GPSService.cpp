#include "SIM5360GPSService.h"

SIM5360GPSService::SIM5360GPSService() {
    responseBuffer = (char*)malloc(responseBufferSize);
}

SIM5360GPSService::~SIM5360GPSService() {
    delete responseBuffer;
}

int SIM5360GPSService::sendCommandAndWaitForResponse(const char* command, uint32_t timeout) {
    serial->print(command);
    memset(responseBuffer, 0, responseBufferSize);

    return getSerialResponse(responseBuffer, timeout);
}

int SIM5360GPSService::getSerialResponse(char* buffer, uint32_t timeout) {
    uint32_t start = millis();
    int read = 0;

    while (true) {
        while (serial->available()) {
            char c = serial->read();
            buffer[read++] = c;
        }

        if (millis() - start >= timeout) {
            break;
        }
    }

    return read;
}

bool SIM5360GPSService::begin(Stream* serial) {
    this->serial = serial;

    sendCommandAndWaitForResponse("ATE0\r\n");
    if (strstr(responseBuffer, "OK") == nullptr) {
        return false;
    }

    sendCommandAndWaitForResponse("AT+CGPS=0\r\n");
    if (strstr(responseBuffer, "OK") == nullptr) {
        return false;
    }

    sendCommandAndWaitForResponse("AT+CGPS=1\r\n");
    if (strstr(responseBuffer, "OK") == nullptr) {
        return false;
    }

    return true;
}

void SIM5360GPSService::run() {
    sendCommandAndWaitForResponse("AT+CGPSINFO\r\n");
    String cleanResponse = cleanupResponse(responseBuffer, "+CGPSINFO:");

    ESP_LOGI(TAG, "%s", cleanResponse.c_str());
    gpsData.parse((char*)cleanResponse.c_str(), cleanResponse.length());
}

String SIM5360GPSService::cleanupResponse(char* data, const char* tobeStriped) {
    String str = String(data);
    str.replace("\n", "");
    str.replace("\r", "");
    str.replace(" ", "");
    str.replace("OK", "");
    str.replace(tobeStriped, "");

    return str.c_str();
}

SIM5360GPSData::SIM5360GPSData():
latitude(0), longitude(0), altitude(0), speed(0) {

}

SIM5360GPSData::~SIM5360GPSData() {

}

int SIM5360GPSData::readUntil(char* buffer, int length, char token, int start) {
    int i = start;
    while (buffer[i] != token && i < length) {
        i++;
    }

    return i + 1;
}

int SIM5360GPSData::getStringSlice(char* haystack, char* dest, int start, int end, char skip) {
    int i = 0;
    while (start < end) {
        if (haystack[start] == skip) {
            start++;
            continue;
        }

        dest[i++] = haystack[start];
        start++;
    }

    return i;
}
void SIM5360GPSData::parse(char* data, int length) {
    char latitude[16] = {0};
    char latPolarity;
    char longitude[16] = {0};
    char lonPolarity;
    char date[8] = {0};
    char utctime[16] = {0};
    char speed[16] = {0};
    char altitude[12] = {0};

    int latEndPosition = readUntil(data, length, ',');
    getStringSlice(data, latitude, 0, latEndPosition - 1, '.');

    int latPolarityEndPosition = readUntil(data, length, ',', latEndPosition + 1);
    getStringSlice(data, &latPolarity, latEndPosition, latPolarityEndPosition - 1);

    int lonEndPosition = readUntil(data, length, ',', latPolarityEndPosition + 1);
    getStringSlice(data, longitude, latPolarityEndPosition, lonEndPosition - 1, '.');

    int lonPolarityEndPosition = readUntil(data, length, ',', lonEndPosition + 1);
    getStringSlice(data, &lonPolarity, lonEndPosition, lonPolarityEndPosition - 1);

    int dateEndPosition = readUntil(data, length, ',', lonPolarityEndPosition + 1);
    getStringSlice(data, date, lonPolarityEndPosition, dateEndPosition - 1);

    int timeEndPosition = readUntil(data, length, ',', dateEndPosition + 1);
    getStringSlice(data, utctime, dateEndPosition, timeEndPosition - 1);

    int altitudeEndPosition = readUntil(data, length, ',', timeEndPosition + 1);
    getStringSlice(data, altitude, timeEndPosition, altitudeEndPosition - 1, '.');

    int speedEndPosition = readUntil(data, length, ',', altitudeEndPosition + 1);
    getStringSlice(data, speed, altitudeEndPosition, speedEndPosition - 1, '.');


    this->latitude = strtoull(latitude, NULL, 10);
    this->longitude = strtoull(longitude, NULL, 10);
    this->altitude = strtoull(altitude, NULL, 10);
    this->speed = strtoull(speed, NULL, 10);

    if (latPolarity == 'S') {
        this->latitude = this->latitude * (-1);
    }

    if (lonPolarity == 'W') {
        this->longitude = this->longitude * (-1);
    }
}