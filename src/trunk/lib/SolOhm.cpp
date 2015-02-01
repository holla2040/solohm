#include "Energia.h"
#include <stdio.h> 

#include "solohm.h"
#include "WiFi.h"
#include "WiFiClient.h"

#include "files.h"
#include "filedefs.h"

#define HTTP_INDEX              47
#define HTTP_STATUS             723
#define HTTP_TEST               495
#define HTTP_LOADSWEEP_JSON     1499
#define HTTP_LOADSWEEP_CSV      1389
#define HTTP_LOADOFF            778
#define HTTP_LOADMPPT           912
#define HTTP_LOADUP             692
#define HTTP_LOADDOWN           903


WiFiServer  tcp(23);
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

    pinMode(INDICATORLED,OUTPUT);

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

    indicatorLEDTimeout = 0;
    statusTimeout = 0;
    statusBroadcastInterval = 5000;
    milliseconds64 = 0;

    consoleBufferIndex = 0;
    tcpBufferIndex = 0;

    strcpy(mode,"manual");

    Serial.println("solohm setup");
}

void SolOhm::consoleLoop() {
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
        
void SolOhm::tcpServerLoop() {
    char c;

    WiFiClient tcpClient = tcp.available();
    if (tcpClient) {
        if (tcpClient.available()) {
            c = tcpClient.read();
            if (c == '\n') {
                tcpBuffer[tcpBufferIndex] = 0x00;
                Serial.print("tcp:   ");
                Serial.println(tcpBuffer);
                tcpClient.println(tcpBuffer);
                tcpBufferIndex = 0;
            } else {
                tcpBuffer[tcpBufferIndex++] = c;
            }
        }

        if (millis64() > statusBroadcastTimeout) {
            char buffer[255];
            statusGet(buffer);
            tcpClient.print(buffer);
        }
    }
}

void SolOhm::httpServerLoop() {
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

    Serial.print(millis());
    Serial.print(" ");
    Serial.println(httpBuffer);

    method    = strtok(httpBuffer," ");
    pathquery = strtok(NULL," ");
    version   = strtok(NULL," ");
 /*
    Serial.println(method);
    Serial.println(pathquery);
*/

    path      = strtok(pathquery,"?");
    query     = strtok(NULL," ");
/*
    Serial.println(path);
    Serial.println(query);
*/
    
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

#ifdef DEBUG
    Serial.print("pathHash ");
    Serial.print(s);
    Serial.print(" ");
    Serial.println(rv,DEC);
#endif

    return rv;
}

void SolOhm::httpDispatch(WiFiClient httpClient, char *path, char *query) {
    char statusCode[50] = "HTTP/1.1 200 OK";
    char contentType[50] = "Content-Type: text/html";
    char messageBodyHeader[1024] = "<html><body>";
    char messageBody[28000] = "";
    char messageBodyFooter[1024] = "</body></html>";

    switch (pathHash(path)) {
        case HTTP_INDEX:
            strcpy(messageBody,(char *)index_html);
            break;
        case HTTP_STATUS:
            statusGet(messageBody); 
            break;
        case HTTP_STATUS_JSON:
            strcpy(contentType,"application/json");
            strcpy(messageBodyHeader,"");
            statusGetJSON(messageBody); 
            strcpy(messageBodyFooter,"");
            break;
        case HTTP_LOADSWEEP_JSON:
            strcpy(contentType,"application/json");
            strcpy(messageBodyHeader,"");
            loadSweepJSON(messageBody); 
            strcpy(messageBodyFooter,"");
            break;
        case HTTP_LOADSWEEP_CSV:
            strcpy(contentType,"text/csv");
            strcpy(messageBodyHeader,"");
            loadSweepCSV(messageBody); 
            strcpy(messageBodyFooter,"");
            break;
        case HTTP_LOADMPPT:
            loadMPPT(messageBody); 
            break;
        case HTTP_LOADOFF:
            loadOff(messageBody); 
            break;
        case HTTP_LOADUP:
            loadUp(messageBody); 
            break;
        case HTTP_LOADDOWN:
            loadDown(messageBody); 
            break;
        case HTTP_TEST:
           // strcpy(messageBody,(char *)example_min_html);
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
    }
    httpClient.println(messageBody);
    if (strlen(messageBodyFooter)) {
        httpClient.println(messageBodyFooter);
    }

    httpClient.stop();
}

void SolOhm::statusGet(char *body) {
    char buffer[50];
    sprintf(buffer,"%d %4d %u.%02u ",millis(),daysensor,(int)vbatt1,(int)(100*vbatt1 - 100*trunc(vbatt1))); strcpy(body,buffer);

    sprintf(buffer,"%u.%02u ",(int)vbatt2,(int)(100*vbatt2 - 100*trunc(vbatt2)));
    strcat(body,buffer);

    sprintf(buffer,"%u.%02u ",(int)vbatt3,(int)(100*vbatt3 - 100*trunc(vbatt3)));
    strcat(body,buffer);

    sprintf(buffer,"%u.%02u ",(int)v3p3,(int)(100*v3p3 - 100*trunc(v3p3)));
    strcat(body,buffer);

    sprintf(buffer,"%u.%02u ",(int)v5p0,(int)(100*v5p0 - 100*trunc(v5p0)));
    strcat(body,buffer);

    sprintf(buffer,"%u.%02u ",(int)vpanel,(int)(100*vpanel - 100*trunc(vpanel)));
    strcat(body,buffer);

    sprintf(buffer,"%u.%02u\n",(int)ipanel,(int)(100*ipanel - 100*trunc(ipanel)));
    strcat(body,buffer);

}

void SolOhm::statusGetJSON(char *body) {
    char buffer[100];

    sprintf(buffer,"{\n  \"uptime\":%d,\n",millis()/1000);
    strcpy(body,buffer);

    sprintf(buffer,"  \"daysensor\":%d,\n",daysensor);
    strcat(body,buffer);

    sprintf(buffer,"  \"dac\":%d,\n",dac);
    strcat(body,buffer);

    sprintf(buffer,"  \"mode\":\"%s\",\n",mode);
    strcat(body,buffer);

    sprintf(buffer,"  \"vbatt\":%u.%02u,\n",(int)vbatt,(int)(100*vbatt - 100*trunc(vbatt)));
    strcat(body,buffer);
    
    sprintf(buffer,"  \"vbatt1\":%u.%02u,\n",(int)vbatt1,(int)(100*vbatt1 - 100*trunc(vbatt1)));
    strcat(body,buffer);

    sprintf(buffer,"  \"vbatt2\":%u.%02u,\n",(int)vbatt2,(int)(100*vbatt2 - 100*trunc(vbatt2)));
    strcat(body,buffer);

    sprintf(buffer,"  \"vbatt3\":%u.%02u,\n",(int)vbatt3,(int)(100*vbatt3 - 100*trunc(vbatt3)));
    strcat(body,buffer);

    sprintf(buffer,"  \"v3p3\":%u.%02u,\n",(int)v3p3,(int)(100*v3p3 - 100*trunc(v3p3)));
    strcat(body,buffer);

    sprintf(buffer,"  \"v5p0\":%u.%02u,\n",(int)v5p0,(int)(100*v5p0 - 100*trunc(v5p0)));
    strcat(body,buffer);

    sprintf(buffer,"  \"rload\":%u.%02u,\n",(int)rload,(int)(100*rload - 100*trunc(rload)));
    strcat(body,buffer);

    sprintf(buffer,"  \"vpanel\":%u.%02u,\n",(int)vpanel,(int)(100*vpanel - 100*trunc(vpanel)));
    strcat(body,buffer);

    sprintf(buffer,"  \"ipanel\":%u.%02u\n}",(int)ipanel,(int)(100*ipanel - 100*trunc(ipanel)));
    strcat(body,buffer);
}

void SolOhm::loop() {
    indicatorLoop();
    statusLoop();
    consoleLoop();
    httpServerLoop();
    tcpServerLoop();
}

uint64_t SolOhm::millis64() {
    return milliseconds64;
}

void SolOhm::indicatorLoop() {
    if (millis64() > indicatorLEDTimeout) {
        indicatorLEDToggle();
        indicatorLEDTimeout = millis64() + INDICATORLEDTIMEOUT;
    }
}

void SolOhm::statusLoop() {
    float v;
    if (millis64() > statusTimeout) {
        vbatt1    = dmmRead(VBATT1);
        v         = dmmRead(VBATT2);
        vbatt2    = v - vbatt1;
        vbatt     = dmmRead(VBATT3);
        vbatt3    = vbatt - v;
        v3p3      = dmmRead(V3P3);
        v5p0      = dmmRead(V5P0);
        statusTimeout = millis64() + STATUSTIMEOUT;
    }
    daysensor = dmmRead(DAYSENSOR);
    vpanel    = dmmRead(VPANEL);
    v         = dmmRead(IPANEL) - 0.06;
    ipanel    = (abs(v) < 0.02)?0.00001:v;
    rload     = vpanel/ipanel;

    statusBroadcast();
}

void SolOhm::statusBroadcast() {
    float d,v;
    char buffer[255];
    if (millis64() > statusBroadcastTimeout) {
        statusBroadcastTimeout = millis64() + statusBroadcastInterval;
        statusGet(buffer);
        //udpBroadcast(buffer);
        Serial.print(buffer);
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
    tcp.begin();
    http.begin();
} 

// socat UDP4-RECVFROM:8380,broadcast,fork STDOUT
void SolOhm::udpBroadcast(char *s) {
    udp.beginPacket("255.255.255.255",consolePort+1);
    udp.print(s);
    udp.endPacket();
}

void SolOhm::dacSet(uint16_t v) {
    dac = v;
    Wire.beginTransmission(DACADDRESS);
    Wire.write(MCP4726_CMD_WRITEDAC);
    Wire.write(v / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
    Wire.write((v % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
    Wire.endTransmission();
}

uint16_t SolOhm::dacGet() {
    return dac;
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
  if (channel != channelLast) {
    delay(100);
  }
  channelLast = channel;
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
        case V5P0:      
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

void SolOhm::indicatorLEDToggle() {
    digitalWrite(INDICATORLED,!digitalRead(INDICATORLED));
}

void SolOhm::loadSweepJSON(char *body) {
    int i;
    float p;
    char buffer[25];
    loadSweep();

    sprintf(buffer,"[");
    strcpy(body,buffer);

    for (i = 0; i < SWEEPLENGTH; i++) {
        p = sweepVoltages[i] * sweepCurrents[i];
        sprintf(buffer,"[%u.%02u,%u.%02u,%u.%02u],",(int)sweepVoltages[i],(int)(100*sweepVoltages[i] - 100*trunc(sweepVoltages[i])),(int)sweepCurrents[i],(int)(100*sweepCurrents[i] - 100*trunc(sweepCurrents[i])),(int)p,(int)(100*p - 100*trunc(p)));
        strcat(body,buffer);
    }

    body[strlen(body) - 1] = 0x00; // pull ', ' chars off body

    sprintf(buffer,"]");
    strcat(body,buffer);
}

void SolOhm::loadSweep() {
    int i;
    uint16_t d,dold;
    
    dold = dacGet();
    for (i = 0; i < SWEEPLENGTH; i++) {
        d = i * SWEEPSTEP + SWEEPDACSTART;
        dacSet(d);
        delay(3);
        sweepVoltages[i] = dmmRead(VPANEL);
        sweepCurrents[i] = dmmRead(IPANEL);
        sweepDACs[i]     = d;
    }
    dacSet(dold);
}

void SolOhm::loadSweepCSV(char *body) {
    int i;
    float f;
    char buffer[50];
    loadSweep();

    sprintf(buffer,"dac,voltage,current,rload,power\n");
    strcpy(body,buffer);

    for (i = 0; i < SWEEPLENGTH; i++) {
        sprintf(buffer,"%d,",sweepDACs[i]);
        strcat(body,buffer);
        sprintf(buffer,"%u.%02u,",(int)sweepVoltages[i],(int)(100*sweepVoltages[i] - 100*trunc(sweepVoltages[i])));
        strcat(body,buffer);
        sprintf(buffer,"%u.%02u,",(int)sweepCurrents[i],(int)(100*sweepCurrents[i] - 100*trunc(sweepCurrents[i])));
        strcat(body,buffer);
        f = sweepVoltages[i]/sweepCurrents[i];
        sprintf(buffer,"%u.%02u,",(int)f,(int)(100*f - 100*trunc(f)));
        strcat(body,buffer);
        f = sweepVoltages[i]*sweepCurrents[i];
        sprintf(buffer,"%u.%02u\n",(int)f,(int)(100*f - 100*trunc(f)));
        strcat(body,buffer);
    }

    //body[strlen(body) - 1] = 0x00; // pull ', ' chars off body
    strcat(body,"\n");
}

void SolOhm::loadOff(char *body) {
#ifdef DEBUG
    Serial.println("loadOff");
#endif

    dacSet(0);
    strcpy(body,"dac:0");
}

void SolOhm::loadMPPT(char *body) {
    int i;
    float p,pmax;
    uint16_t d,dmax;
    char buffer[50];
    dmax = 0;
    pmax = 0.0;
    
    for (i = 0; i < SWEEPLENGTH; i++) {
        d = i * SWEEPSTEP + SWEEPDACSTART;
        dacSet(d);
        delay(3);
        p = dmmRead(VPANEL) * dmmRead(IPANEL);

#ifdef DEBUG
        Serial.print(d);
        Serial.print(" ");
        Serial.print(p,2);
        Serial.print(" ");
        Serial.println(pmax,2);
#endif

        if (p > pmax) {
            dmax = d;
            pmax = p;
        }
    }
    dacSet(dmax);
    sprintf(buffer,"dac:%u",d);
    strcat(body,buffer);
}

void SolOhm::loadUp(char *body) {
    char buffer[50];
    uint16_t d;
    d = dacGet();
    d -= 25;
    if (d < 50) {
        d = 0;
    } 
    sprintf(buffer,"dac:%u",d);
    strcat(body,buffer);
    dacSet(d);
}

void SolOhm::loadDown(char *body) {
    char buffer[50];
    uint16_t d;
    d = dacGet();
    d += 25;
    if (d > 3000) {
        d = 4000;
    } 
    sprintf(buffer,"dac:%u",d);
    strcat(body,buffer);
    dacSet(d);
}
