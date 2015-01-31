#ifndef SOLOHM_H
#define SOLOHM_H 1

#include "../../libraries/Wire/Wire.h"
#include "../../libraries/WiFi/WiFi.h"
#include "../../libraries/WiFi/WiFiServer.h"

#include <stdint.h>

#define SWEEPLENGTH 1000
#define SWEEPSTEP   (4096/SWEEPLENGTH)

class SolOhm {
    public:
        SolOhm(){};
        void        setup();
        void        setup(char *ssid, char *password);
        void        setup(char *ssid, char *password, uint16_t port);
        void        dacSet(uint16_t v);
        float       dmmRead(uint8_t channel);
        void        udpBroadcast(char *s);
        void        loop();
        uint64_t    millis64();

    private:
        void        setRTCRegister(unsigned char registerNumber, unsigned char value);
        uint8_t     getRTCRegister(unsigned char registerNumber);
        uint16_t    amuxRead(uint8_t channel);
        char        wifiSSID[50];
        char        wifiPassword[50];
        uint16_t    consolePort;
        void        indicatorLEDToggle();

        void        statusLoop();
        uint32_t    statusTimeout;

        void        indicatorLoop();

        WiFiUDP     udp;
        uint32_t    indicatorLEDTimeout;
        uint32_t    statusBroadcastInterval;
        uint32_t    statusBroadcastTimeout;
        void        statusBroadcast();

        char        consoleBuffer[50];
        uint8_t     consoleBufferIndex;
        void        consoleLoop();

        char        tcpBuffer[255];
        uint8_t     tcpBufferIndex;
        void        tcpServerLoop();

        char        httpBuffer[255];
        uint8_t     httpBufferIndex;
        void        httpServerLoop();
        void        httpProcessRequest(WiFiClient httpClient);
        void        httpDispatch(WiFiClient httpClient, char *path, char *query);
        uint32_t    pathHash(char *s);

        float       v3p3;
        float       v5p0;
        float       vbatt;
        float       vbatt1;
        float       vbatt2;
        float       vbatt3;
        uint16_t    daysensor;
        float       vpanel;
        float       ipanel;
        float       rload;

        void        statusGet(char *body);
        void        statusGetJSON(char *body);
        void        statusGetJSONList(char *body);
        uint8_t     channelLast;
        void        loadSweep();
        void        loadSweepJSON(char *body);

        float       sweepVoltages[SWEEPLENGTH];
        float       sweepCurrents[SWEEPLENGTH];

        char        mode[10];
}; 

#define VPANELSCALED    A2
#define IPANELSCALED    A1

#define V5SWENABLE      03

#define AMUXS0          05
#define AMUXS1          27
#define AMUXS2          28
#define AMUXOUT         A3


#define VBATT1SCALE  4.136/3057.0
#define VBATT2SCALE  8.279/3351.0
#define VBATT3SCALE 12.413/3130.0
#define V3P3SCALE    3.32/3836.0
#define V5SCALE      5.00/3712.0
#define VPANELSCALE 25.44/2081.0
#define IPANELM     -0.0025316
#define IPANELB      9.6075949

#define INDICATORLEDTIMEOUT 1000
#define STATUSTIMEOUT       5000

#define CONSOLEPORT  8379

enum {
    DAYSENSORSCALED,
    VBATT1SCALED,
    VBATT2SCALED,
    VBATT3SCALED,
    V3P3SCALED,
    V5SWSCALED,
};

enum {
    DAYSENSOR,
    VBATT1,
    VBATT2,
    VBATT3,
    V3P3,
    V5P0,
    IPANEL,
    VPANEL
};

#define INDICATORLED     29

#define THERMOCOUPLECS  19

#define IRRECEIVER      32

#define DISPLAYDC       30
#define DISPLAYCS       18
#define DISPLAYRESET    31

#define SDCD            17
#define SDCS            08

#define SPI_MOSI        15
#define SPI_MISO        14
#define SPI_CLK         07

#define I2C_SCL         09
#define I2C_SDA         10

#define BATTCHARGING     04
#define BATTCHARGEENABLE 13

/* from adafruit MCP4726 lib */
#define MCP4726_CMD_WRITEDAC            (0x40)  // Writes data to the DAC
#define MCP4726_CMD_WRITEDACEEPROM      (0x60)  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)

#define DACADDRESS 0x60




/* RTC defines */

// from https://github.com/sridharrajagopal/DSRTCLib
// indices within the rtc_bcd[] buffer

#define DSRTCLib_TR     0x10
#define DSRTCLib_SEC    0
#define DSRTCLib_MIN    1
#define DSRTCLib_HR 2
#define DSRTCLib_DOW    3
#define DSRTCLib_DATE     4
#define DSRTCLib_MTH    5
#define DSRTCLib_YR 6

#define DSRTCLib_BASE_YR        2000

#define DSRTCLib_CTRL_ID        B1101000

// Define register bit masks

#define DSRTCLib_CLOCKHALT  B10000000

#define DSRTCLib_LO_BCD     B00001111
#define DSRTCLib_HI_BCD     B11110000

#define DSRTCLib_HI_SEC     B01110000
#define DSRTCLib_HI_MIN     B01110000
#define DSRTCLib_HI_HR      B00110000
#define DSRTCLib_LO_DOW     B00000111
#define DSRTCLib_HI_DATE        B00110000
#define DSRTCLib_HI_MTH     B00110000
#define DSRTCLib_HI_YR      B11110000

#define DSRTCLib_ARLM1      0x07
#define DSRTCLib_ARLM1_LO_SEC   B00001111
#define DSRTCLib_ARLM1_HI_SEC   B01110000
#define DSRTCLib_ARLM1_LO_MIN   B01110000
#define DSRTCLib_ARLM1_HI_MIN   B00001111

#define DSRTCLib_SP         0x0E
#define DSRTCLib_SP_EOSC        B10000000
#define DSRTCLib_SP_RS2     B00010000
#define DSRTCLib_SP_RS1     B00001000
#define DSRTCLib_SP_INTCN       B00000100
#define DSRTCLib_SP_A2IE        B00000010
#define DSRTCLib_SP_A1IE        B00000001

#define DSRTCLib_STATUS     0x0F
#define DSRTCLib_STATUS_OSF B10000000
#define DSRTCLib_STATUS_A2F B00000010
#define DSRTCLib_STATUS_A1F B00000001




#endif
