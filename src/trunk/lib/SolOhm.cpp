#include "Energia.h"
#include <stdio.h> 

#include "solohm.h"
#include "WiFi.h"
#include "WiFiClient.h"


// WiFiServer  tcp(23);
WiFiServer  http(80);

/* for tcp connection testing use 'stty cbreak' to disable tty in buffering */

extern "C" {
    void milliseconds64Update(uint32_t v);
    uint64_t milliseconds64;

    void milliseconds64Update(uint32_t v) {
        milliseconds64++;
    }
}

void SolOhm::setup() {
    float v;
    char buffer[10];
    Serial.begin(115200);
    Wire.begin();

// ADC setup
    pinMode(VPANELSCALED,INPUT);
    pinMode(IPANELSCALED,INPUT);
    pinMode(AMUXOUT,INPUT);

    pinMode(AMUXS0,OUTPUT);
    pinMode(AMUXS1,OUTPUT);
    pinMode(AMUXS2,OUTPUT);
    pinMode(V5SWENABLE,OUTPUT);
    digitalWrite(V5SWENABLE,1);

    pinMode(ACTIVITYLED,OUTPUT);

// DAC setup
    Wire.beginTransmission(DACADDRESS);
    Wire.write(MCP4726_CMD_WRITEDACEEPROM);
    Wire.write(0x10);        // 1kohm to ground, 0 output
    Wire.write(0x00);
    Wire.endTransmission();

// RTC setup
    setRTCRegister(DSRTCLib_SP, getRTCRegister(DSRTCLib_SP) & !DSRTCLib_SP_EOSC); //start RTC
    setRTCRegister(DSRTCLib_TR,0xA6); // no diode, 2k resistor;

    registerSysTickCb(milliseconds64Update);

    activityLEDTimeout = 0;
    statusBroadcastInterval = 1000;
    milliseconds64 = 0;

    consoleBufferIndex = 0;
    tcpBufferIndex = 0;


    Serial.println("solohm setup");

}

void SolOhm::consoleProcess() {
    int c;
    if (Serial.available()) {
        c = Serial.read();
        consoleBuffer[consoleBufferIndex++] = c;
        if (c == '\n') {
            consoleBuffer[consoleBufferIndex] = 0x00;
            Serial.println(consoleBuffer);
            consoleBufferIndex = 0;
        }
    }
}
        
void SolOhm::tcpServerProcess() {
    char c;
/*
    WiFiClient tcpClient = tcp.available();
    if (tcpClient) {
        if (tcpClient.available()) {
            c = tcpClient.read();
            if (c == '\n') {
                tcpBuffer[tcpBufferIndex] = 0x00;
                Serial.print("tcp:   ");
                Serial.println(tcpBuffer);
                tcp.println(tcpBuffer);
                tcpBufferIndex = 0;
            } else {
                tcpBuffer[tcpBufferIndex++] = c;
            }
        }
    }
*/
}

void SolOhm::httpServerProcess() {
    char c;

    WiFiClient httpClient = http.available();
    if (httpClient) {
        while (httpClient.connected()) {
            if (httpClient.available()) {
                c = httpClient.read();
                if (c == '\n') {
                    httpBuffer[httpBufferIndex] = 0x00;
                    httpProcessRequest(httpClient);
                    httpBufferIndex = 0;
                    break;
                } else {
                    httpBuffer[httpBufferIndex++] = c;
                }
            }
        }
    }
}

/* ref http://code.tutsplus.com/tutorials/http-the-protocol-every-web-developer-must-know-part-1--net-31177 */
void SolOhm::httpProcessRequest(WiFiClient httpClient) {
    char *method;
    char *pathquery;
    char *path;
    char *query;
    char *version;

    //Serial.println(httpBuffer);

    method    = strtok(httpBuffer," ");
    pathquery = strtok(NULL," ");
    version   = strtok(NULL," ");
    Serial.println(method);
    Serial.println(pathquery);

    path      = strtok(pathquery,"?");
    query     = strtok(NULL," ");
    Serial.println(path);
    Serial.println(query);
    
    httpDispatch(httpClient,path,query);
}

uint32_t SolOhm::pathHash(char *s) {
    uint32_t rv = 0;
/*
    Serial.print("pathHash ");
    Serial.println(s);
*/
    while (*s) {
        rv += *s++;
    }
    Serial.print("pathHash ");
    Serial.print(s);
    Serial.print(" ");
    Serial.println(rv,DEC);

    return rv;
}

void SolOhm::httpDispatch(WiFiClient httpClient, char *path, char *query) {
    char statusCode[50] = "HTTP/1.1 200 OK";
    char contentType[50] = "Content-Type: text/html";
    char messageBodyHeader[1024] = "<html><body>";
    char messageBody[1024] = "";
    char messageBodyFooter[1024] = "</body></html>";

    switch (pathHash(path)) {
        case PATH_INDEX:
            strcat(messageBody,"solohm");
            break;
        case PATH_STATUS:
            strcpy(messageBody,"sup?");
            break;
        default:
            strcpy(statusCode,"HTTP/1.1 404 Not Found");
            strcpy(messageBodyHeader,"<html><head><title>404 Not Found</title></head><body>");
            strcpy(messageBody,"<h1>Not Found</h1><p>The requested URL ");
            strcat(messageBody,path); 
            strcat(messageBody," was not found on this server.</p><hr></body></html>");
    }

    httpClient.println(statusCode);
    httpClient.println(contentType);
    httpClient.println();
    if (strlen(messageBodyHeader)) {
        httpClient.println(messageBodyHeader);
        httpClient.println(messageBody);
        httpClient.println(messageBodyFooter);
    }

    httpClient.stop();
}


void SolOhm::loop() {
    statusUpdate();
    statusBroadcast();
    consoleProcess();
    httpServerProcess();
}

uint64_t SolOhm::millis64() {
    return milliseconds64;
}

void SolOhm::statusUpdate() {
    if (millis64() > activityLEDTimeout) {
        activityLEDToggle();
        activityLEDTimeout = millis64() + ACTIVITYLEDTIMEOUT;
    }
}

void SolOhm::statusBroadcast() {
    float d,v;
    char buffer[10];
    if (millis64() > statusBroadcastTimeout) {
        statusBroadcastTimeout = millis64() + statusBroadcastInterval;
        d = dmmRead(DAYSENSOR);
        v = dmmRead(VBATT3);
v = 1.23456;
        // sprintf(buffer,"%d %4d %u.%02u\n",millis(),(int)d,(int)v,(int)(100*v - 100*trunc(v)));
Serial.println(v,6);
        //sprintf(buffer,"%d %4d %.2f\n",millis(),d,v);
        sprintf(buffer,"%.2f",v);
        udpBroadcast(buffer);
        Serial.println(buffer);
    }
}

void SolOhm::setRTCRegister(unsigned char registerNumber, unsigned char value) {
    Wire.beginTransmission(DSRTCLib_CTRL_ID);
    Wire.write(registerNumber); // set register pointer
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t SolOhm::getRTCRegister(unsigned char registerNumber) {
    Wire.beginTransmission(DSRTCLib_CTRL_ID);
    Wire.write(registerNumber);
    Wire.endTransmission();
    Wire.requestFrom(DSRTCLib_CTRL_ID, 1);
    return Wire.read();
}

void SolOhm::setup(char *ssid, char *password) {
    setup(ssid,password,CONSOLEPORT);
}

void SolOhm::setup(char *ssid, char *password, uint16_t port) {
    setup();

    strcpy(wifiSSID,ssid);
    strcpy(wifiPassword,password);
    consolePort = port;
    Serial.print("wifi: connecting to AP ");
    Serial.println(wifiSSID);
    WiFi.begin(wifiSSID,wifiPassword);
    while ( WiFi.status() != WL_CONNECTED) {
        delay(300);
    }
    Serial.print("wifi: getting IP address ");
    while (WiFi.localIP() == INADDR_NONE) {
        delay(300);
    }
    Serial.println(WiFi.localIP());
    udp.begin(consolePort);
    // tcp.begin();
    http.begin();
} 

// socat UDP4-RECVFROM:8380,broadcast,fork STDOUT
void SolOhm::udpBroadcast(char *s) {
    udp.beginPacket("255.255.255.255",consolePort+1);
    udp.print(s);
    udp.endPacket();
}

void SolOhm::dacSet(uint16_t v) {
    Wire.beginTransmission(DACADDRESS);
    Wire.write(MCP4726_CMD_WRITEDAC);
    Wire.write(v / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
    Wire.write((v % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
    Wire.endTransmission();
}

uint16_t SolOhm::amuxRead(uint8_t channel) {
  switch (channel) {
  case DAYSENSORSCALED:      
    digitalWrite(AMUXS0,0);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,0);
    break;
  case VBATT1SCALED:      
    digitalWrite(AMUXS0,1);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,0);
    break;
  case VBATT2SCALED:      
    digitalWrite(AMUXS0,0);
    digitalWrite(AMUXS1,1);
    digitalWrite(AMUXS2,0);
    break;
  case VBATT3SCALED:      
    digitalWrite(AMUXS0,1);
    digitalWrite(AMUXS1,1);
    digitalWrite(AMUXS2,0);
    break;
  case V3P3SCALED:      
    digitalWrite(AMUXS0,0);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,1);
    break;
  case V5SWSCALED:      
    digitalWrite(AMUXS0,1);
    digitalWrite(AMUXS1,0);
    digitalWrite(AMUXS2,1);
    break;
  }
  delay(100);
  return analogRead(AMUXOUT);
}

float SolOhm::dmmRead(uint8_t channel) {
    float rv;
    switch (channel) {
        case DAYSENSOR:      
            return amuxRead(DAYSENSORSCALED);
            break;
        case VBATT1:      
            return float(amuxRead(VBATT1SCALED))*VBATT1SCALE;
            break;
        case VBATT2:      
            return float(amuxRead(VBATT2SCALED))*VBATT2SCALE;
            break;
        case VBATT3:      
            rv = float(amuxRead(VBATT3SCALED))*VBATT3SCALE;
            return rv;
            break;
        case V3P3:      
            return float(amuxRead(V3P3SCALED))*V3P3SCALE;
            break;
        case V5:      
            return float(amuxRead(V5SWSCALED))*V5SCALE;
            break;
        case IPANEL:
            return IPANELM * float(analogRead(IPANELSCALED)) + IPANELB;
            break;
        case VPANEL:      
            return float(analogRead(VPANELSCALED))*VPANELSCALE;
            break;
    }
    return 0.0;
}

void SolOhm::activityLEDToggle() {
    digitalWrite(ACTIVITYLED,!digitalRead(ACTIVITYLED));
}
