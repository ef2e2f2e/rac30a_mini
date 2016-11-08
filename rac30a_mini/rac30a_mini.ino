//******************************
//RAC30a  - release 1.0
//******************************


#define ARDUINODUE
//#define ARDUINOADK
//#define ARDUINOMINI
//#define NODEMCU


#include "CodesManager.h"
#include "Keypad.h"
#include <Arduino.h>


#ifdef ARDUINOMINI
#include <SoftwareSerial.h>
SoftwareSerial Serial1(10, 11); // RX, TX
#endif
#ifdef NODEMCU
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
const char* ssid = "andromeda";
const char* password = "Enricoeandrea0!";
ESP8266WebServer server(80);
IPAddress ip(192, 168, 0, 51); 
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
//#include <SoftwareSerial.h>
//SoftwareSerial Serial1(13, 15); // RX, TX
  
void handleRoot(){
  Serial.println("Ricevuta richiesta HTTP");
  String content = "<html><body><H2>Hello, Remote access control service. Use our API calls to access the services.</H2><br>";
  content = content + "Request parameters: <br> req(=access),key,authkey,opcode,extradata <br> or req(=getkey),usr,pwd</body></html>";
  server.send(200, "text/html", content);
}

#define RACREMOTEURL "http://http://astropointing.appspot.com/"
#define RACREMOTEKEY "krd4-R7y5-SG2M-Kf0c-57Z2"

boolean httpPost(String payload){
   HTTPClient http;
   http.begin(RACREMOTEURL);
   http.addHeader("Content-Type", "application/json");
   // a simple payload, for doc on payload format see: https://docs.internetofthings.ibmcloud.com/messaging/payload.html
   Serial.print("POST payload: "); Serial.println(payload);
   int httpCode = http.POST(payload);
   Serial.print("HTTP POST Response: "); Serial.println(httpCode); // HTTP code 200 means ok 
   http.end();
}
boolean accessRemoteLog(String channel, String timex, String code, String esito) {
  //String json = "{ \"channel\" : \"" + channel + "\", \"time\" : \"" + timex + "\",\"code\" : \"" + code + "\",\"esito\" : \"" + esito + "\"}";
  String x = "ch=" + channel + "&time="+ timex +"&code=" + code + "&esito=" + esito + "&key=" + RACREMOTEKEY;  
  httpPost(x);
}

#endif



//---------------------------------------------------
// #DEFINE SECTION
//---------------------------------------------------
//---------------------------------------------------   
#define PINPORTA             13
#define PINPORTONCINO        15
#define OPENPORTPULSEWIDTH   2    //opening impulse duration (in sec)
//---------------------------------------------------
#define MAXLEN_COMMANDFROMSERIAL        20
#define MAXLEN_KEYPADBUFFER             10
#define MAXLEN_SERIALBUFFER             20
#define MAXLEN_SERIALXBUFFER            20
#define MAXLEN__ESITOCOMMANDBUFFER      20
//---------------------------------------------------
#define INPUTMODE_NORMAL 100
#define INPUTMODE_PRG    200
//---------------------------------------------------
#define PHYSICALMEMORYLEN 600
#define PHYSICALMEMORYCODEADDRESS 100
//---------------------------------------------------
#define CHANNELKEYPAD    100
#define CHANNELSERIAL    200
#define CHANNELSERIALX   300




int counterLoop;
CodesManager codici;
Keypad *mtxkpd;
//buffer Keypad
char keypadBuffer[MAXLEN_KEYPADBUFFER];
char authKeypadBuffer[MAXLEN_KEYPADBUFFER];
int len_keypadBuffer;
void keypadBufferReset(){ len_keypadBuffer=0; for (int i=0; i<MAXLEN_KEYPADBUFFER ; i++){ keypadBuffer[i]='\0';}}
void authKeypadBufferReset(){for (int i=0; i<MAXLEN_KEYPADBUFFER ; i++){ authKeypadBuffer[i]='\0';}}
void copyKeypadBufferToButhBuffer(){for (int i=0; i<MAXLEN_KEYPADBUFFER ; i++){ authKeypadBuffer[i]=keypadBuffer[i];}}
//buffer Serial
char serialChannelBuffer[MAXLEN_SERIALBUFFER];
int len_serialChannelBuffer;
void serialChannelBufferReset(){ len_serialChannelBuffer=0; for (int i=0; i<MAXLEN_SERIALBUFFER ; i++){ serialChannelBuffer[i]='\0';}}
#ifndef NODEMCU
//buffer Serialx
char serialxChannelBuffer[MAXLEN_SERIALXBUFFER];
int len_serialxChannelBuffer;
void serialxChannelBufferReset(){ len_serialxChannelBuffer=0; for (int i=0; i<MAXLEN_SERIALXBUFFER ; i++){ serialxChannelBuffer[i]='\0';}}
#endif
//gates 
void apriPorta(){ digitalWrite(PINPORTA, 0);
                  delay(OPENPORTPULSEWIDTH*1000); 
                  digitalWrite(PINPORTA, 1); 
                  #ifdef NODEMCU
                    httpPost("APERTURA PORTA");
                  #endif
}
void apriPortoncino(){digitalWrite(PINPORTONCINO, 0);
                      delay(OPENPORTPULSEWIDTH*1000);
                      digitalWrite(PINPORTONCINO, 1);
                      #ifdef NODEMCU
                        httpPost("APERTURA PORTONCINO");
                      #endif
}





int inputMode;
void manageCommand(String channel, String command);



const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
char keys[ROWS][COLS] = {{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
//{5, 12, 14, 0}{4, 16, 2}
//se NodeMCU circuit:  {2,5,4,0}{12,14,16}
//seArduino2Circuit:   {7,2,3,5}{6,8,4}
byte rowPins[ROWS] = {2, 5, 4, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 14, 16}; //connect to the column pinouts of the keypad
char stringKeypressed[2] = {'\0', '\0'};
int stringKeypressedReset(){stringKeypressed[0]='\0'; stringKeypressed[1]='\0';}


void manageKeypadActivity(){
        //Reading from keyboard
        stringKeypressed[0]=255;
        stringKeypressed[0]=mtxkpd->getKey();
        if (stringKeypressed[0] != 0){
             switch (stringKeypressed[0]) { 
               case '0':
               case '1':
               case '2':
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
               case '8':
               case '9':     Serial.print("!Keypressed: <");
                             Serial.print(stringKeypressed[0]);
                             Serial.println(">");
                             if (len_keypadBuffer > MAXLEN_KEYPADBUFFER){ keypadBufferReset(); Serial.print("CODE_RESET\n");
                             } else {  keypadBuffer[len_keypadBuffer++]=stringKeypressed[0]; Serial.print("CODE_MODIFIED\n");Serial.println(keypadBuffer);
                             }  
                             break;           
               case '*':     keypadBufferReset(); 
                             Serial.print("CODE_RESET\n");
                             break;
               case '#':               
                             if (len_keypadBuffer == 0) Serial.print("CODE_UNCHANGED\n");
                             else { 
                                                                   if (inputMode == INPUTMODE_NORMAL) {
                                                                     if (codici.checkAmmCode(String(keypadBuffer)) == CODESLIB_OK) {
                                                                         Serial.print("Kpad: -> prg mode\n");
                                                                         inputMode = INPUTMODE_PRG;
                                                                         copyKeypadBufferToButhBuffer();
                                                                      } else { 
                                                                        manageCommand(CHANNELKEYPAD,String(keypadBuffer) + String(",") + String(OPEN1_CMDCODE)+String(keypadBuffer));
                                                                        sendFeedback(CHANNELKEYPAD,codici.getEsitoBuffer());
                                                                      }
                                                                      keypadBufferReset();
                                                                   } else {
                                                                      Serial.print("Managing cmd in PRG MODE\n");
                                                                      String tmp=String(authKeypadBuffer)+String(",")+String(keypadBuffer);
                                                                      manageCommand(CHANNELKEYPAD,tmp);
                                                                      sendFeedback(CHANNELKEYPAD,codici.getEsitoBuffer());
                                                                      inputMode = INPUTMODE_NORMAL;
                                                                      keypadBufferReset();
                                                                      authKeypadBufferReset();
                                                                   }
                             }
                             break;
               default:  {Serial.println(stringKeypressed);break;}              
             }
       }
}

char charFromSerial;
int manageSerialChannelActivity(){
      //Reading from Serial (RAC Back End)
      if (Serial.available() > 0) {  
          charFromSerial = Serial.read();
          //a '*' reset the command string. A '#' close the command string
          //a command string is done as *<authcode>,<codecommand><data>#
          switch (charFromSerial) {
            case 0xD: break;
            case '*': serialChannelBufferReset();
                      break;
            case '#': Serial.print("Cmd from serial: <");Serial.print(serialChannelBuffer);Serial.print(">"); Serial.flush();
                      manageCommand(CHANNELSERIAL,serialChannelBuffer);
                      sendFeedback(CHANNELSERIAL,codici.getEsitoBuffer());
                      serialChannelBufferReset();
                      break;
            default : if (len_serialChannelBuffer < MAXLEN_SERIALBUFFER) { serialChannelBuffer[len_serialChannelBuffer++]=charFromSerial;
                      } else {                                             serialChannelBufferReset();
                      }
                      break;
          }
      } 
}

#ifndef NODEMCU
char charFromSerialx;
int manageSerialxChannelActivity(){
    //Reading from Serial (RAC Back End)
    if (Serial1.available() > 0) { 
          charFromSerialx=Serial1.read();
          switch (charFromSerialx) {
                     case 0xD: break;
                     case '*': serialxChannelBufferReset(); break;
            case '#': Serial.print("Cmd from serialx: <");Serial.print(serialxChannelBuffer);Serial.print(">"); Serial.flush();
                      manageCommand(CHANNELSERIALX,serialChannelBuffer);
                      serialChannelBufferReset();
                      sendFeedback(CHANNELSERIALX,codici.getEsitoBuffer());
                      serialxChannelBufferReset();
                      break;
            default : if (len_serialxChannelBuffer < MAXLEN_SERIALXBUFFER-1) { serialxChannelBuffer[len_serialxChannelBuffer]=charFromSerialx;
                                                                               serialxChannelBuffer[len_serialxChannelBuffer+1]='\0';
                                                                               len_serialxChannelBuffer++;
                      } else {                                                 serialxChannelBufferReset();
                      }
                      break;
          }
      }
}
#endif


/**********************************************
From the BE to the FE <Resourse><action><...extra data...>#

"GO1#"            : Open Gate 1 (House door)
"GO2#"            : Open Gate 2 (Building door)
"GEA#"            : Enable automatism for opening building door
"GDA#"            : Enable automatism for opening building door
"KA#"             : Keep Alive
"CE<xxxxx>#"      : enable    code <xxxxx>
"CD<xxxxx>#"      : disable   code <xxxxx>
"CL#"             : retrieve list code. The FE sends response "+CL:code1;code2;...;#"
"CR#"             : reset the codes list



Da RAC FE a RAC BE:
"@...."   : comment, trace, log
"V...."   : Event
"K<x>"    : pressed key <x>                             DONE
"C<xxxx>:<y>"   : code <xxxx> pressed. regognized as valid <1> or not valid <0>   DONE
"GO1"     : opened gate 1                               DONE
"GO2"     : opened gate 2
"GEA"     : enabled automatism
"GDA"     : disabled automatism

"KA"      : keep alive
"+CGL<code1>;<code2>...#"    (get list)
"+CSL<code1>;<code2>...# "  (set list)

***********************************************/



void sendFeedback(int channel,String string){
  if      (channel== CHANNELSERIAL) {Serial.println("Esito comando (da serial) : "); Serial.println(string); }
  #ifndef NODEMCU
  else if (channel== CHANNELSERIALX){Serial.println("Esito comando (da serialx): "); Serial.println(string); Serial1.println(string);}
  #endif
  else if (channel== CHANNELKEYPAD) {Serial.println("Esito comando (da keypad) : "); Serial.println(string);}
}
void manageCommand(int channel, String command){ 
      Serial.println(String("ManageCommand: ch:<") + channel + String("> cmd:<") + command + String(">"));
      String authcode="";
      //The format is: <authcode>,<codecommand><extradata> or #...
      if (command.charAt(0) == '@') {sendFeedback(channel,"OK: a dummy text");return;}
      int firstsc = command.indexOf(',');
      if (firstsc == -1) { sendFeedback(channel,"KO: sintax error");return;
      }
      authcode=command.substring(0,firstsc);
      Serial.print("authcode:<");Serial.print(authcode);Serial.println(">");  
      for (int i = 0; i < authcode.length(); i++) { 
             switch (authcode.charAt(i)){
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '0':  break;
                default : {sendFeedback(channel,"KO: authcode error"); return;}
             } 
      }
      if (command.length() < (firstsc +3)) {sendFeedback(channel,"KO: not valid command"); return;}
      String codecommand=command.substring(firstsc+1,firstsc+3);  
      Serial.print("codecommand:<");Serial.print(codecommand);Serial.println(">");
      Serial.print("----------------------------------------------\n");
      
      if ( (codecommand == ENABLECODE_CMDCODE)  ||
           (codecommand == DISABLECODE_CMDCODE) ||
           (codecommand == CHKCODE_CMDCODE)     ||           
           (codecommand == DISABLEALL_CMDCODE)  ||
           (codecommand == SYSRESET_CMDCODE)    ||
           (codecommand == CHGAMMCODE_CMDCODE)  ||
           (codecommand == CHKAMMCODE_CMDCODE)  ||
           (codecommand == GETCODES_CMDCODE)    ||
           (codecommand == LISTCODES_CMDCODE)   ) {              
           codici.manageCommand(command);
           sendFeedback(channel,String(codici.getEsitoBuffer()));
           return;
      } else if (codecommand == OPEN1_CMDCODE){
            Serial.print("Opening gate1\n"); 
            if (codici.checkCode(authcode) == CODESLIB_OK) {
              apriPorta();
              sendFeedback(channel,"OK:OPENGATE1");
              Serial.println("Apertura porta!");
              return;
            } else {
              //sendFeedback(channel,"KO:OPENGATE1");
              return;        
            }
      } else if (codecommand == OPEN2_CMDCODE) {
            Serial.print("Opening gate2\n"); 
            if (codici.checkCode(authcode) == CODESLIB_OK) {
              apriPortoncino();
              sendFeedback(channel,"OK:OPENGATE2");
              return; 
            } else {
              sendFeedback(channel,"KO:OPENGATE2");
              return;        
            }
      } else if (command.startsWith("KA")==1)  {
                                                      sendFeedback(channel,"OK:KA");
                                                      return;
      } else {  
                                                      sendFeedback(channel,"KO:ERROR");
                                                      return;
      }
      sendFeedback(channel,"KO: not recognized command"); 
      return;
}





void setup() {
      Serial.begin(9600);
      //Empting buffer from serial
      if (Serial.available()  > 0) {  Serial.read();  }   //ATTENZIONE AD EVENTUALI ERRORI (SE NON C'E' CONNESSIONE)
      Serial.println("@Started RAC30a ...");
      #ifndef NODEMCU
      // Serialx channel
      Serial1.begin(9600);
      if (Serial1.available() > 0) {  Serial1.read(); }  //ATTENZIONE AD EVENTUALI ERRORI (SE NON C'E' CONNESSIONE)
      Serial.println("Enabled Serial1");
      serialxChannelBufferReset();
      #endif
      counterLoop=0;
      //gates
      pinMode(PINPORTA, INPUT_PULLUP);   //to avoid problems in the phase of powering the circuit/relays
      pinMode(PINPORTONCINO, INPUT_PULLUP);
      digitalWrite(PINPORTA, 1); 
      digitalWrite(PINPORTONCINO, 1); 
      delay(1000); 
      pinMode(PINPORTA,      OUTPUT);
      pinMode(PINPORTONCINO, OUTPUT);
      //codici
      codici = CodesManager();
      codici.init();
      codici.toSerial();
      codici.resetCodesMemory("3356335591");
      codici.enableCode(String("0823968426"));
      codici.enableCode(String("55555555"));
      //keypad
      mtxkpd = new Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
      //buffers reset
      keypadBufferReset();
      authKeypadBufferReset();
      copyKeypadBufferToButhBuffer();
      serialChannelBufferReset();
      #ifdef NODEMCU
      WiFi.config(ip, gateway, subnet);
      WiFi.begin(ssid, password);
      // Wait for connection
      while (WiFi.status() != WL_CONNECTED) {
           delay(500);
           Serial.println("Wifi connecting");
      }
      Serial.println("Wifi connected");
      Serial.println(ssid);
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());  
      server.on("/", handleRoot);
      //server.on("/api", handleAPI);
      //server.on("/enable", enableAPI);
      //server.on("/disable", disableAPI);
      //server.onNotFound(handleNotFound);
      //here the list of headers to be recorded
      server.begin();
      Serial.println("HTTP server started");
      #endif
      //initilization control variables
      inputMode = INPUTMODE_NORMAL;
      //apriPorta();
      //apriPortoncino();
      Serial.println("@Starting loop ...");
}


void loop() {
   counterLoop++;
   //one keypressed a time
   manageKeypadActivity();
   manageSerialChannelActivity();
   #ifndef NODEMCU
   //manageSerialxChannelActivity();
   #endif
   #ifdef NODEMCU
   server.handleClient();
   #endif

}
    



