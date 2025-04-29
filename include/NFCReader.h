// NFCReader.h
#ifndef NFCREADER_H
#define NFCREADER_H

#include <SPI.h>
#include <MFRC522.h>

#define SCK_PIN 10
#define MISO_PIN 12
#define MOSI_PIN 11
#define SS_PIN 9
#define RST_PIN 13

class NFCReader {
public:
    NFCReader(uint8_t ssPin = 9, uint8_t rstPin = 13);
    void begin();
    bool readCard();
    String getUID() const;
    String getCardType() const;
    String getMappedRoom() const;

private:
    MFRC522 mfrc522;
    String currentUID;
    String cardType;
    String mapUIDToRoom(const String& uid) const;
};

#endif