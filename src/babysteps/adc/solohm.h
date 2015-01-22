#ifndef SOLOHM_H
#define SOLOHM_H 1

#define VPANELSCALED    A2
#define IPANELSCALED    A1

#define V5SWENABLE      03

#define AMUXS0          05
#define AMUXS1          27
#define AMUXS2          28
#define AMUXOUT         24

enum {
    DAYSENSORSCALED,
    VBATT1SCALED,
    VBATT2SCALED,
    VBATT3SCALED,
    V3P3SCALED,
    V5SWSCALED
};

#define ACTIVITYLED     29

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

#endif
