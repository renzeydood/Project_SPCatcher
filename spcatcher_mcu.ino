#include <Streaming.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include "buttonsequence.h"
#include "states.h"

//Compiler directives to Enable/Disable Debug Mode
#define DEBUG_MODE //Comment to disable debug mode
#ifdef DEBUG_MODE
#define D_BEGIN(x) Serial.begin(x)
#define D_PRINT(x) Serial.print(x)
#define D_PRINTLN(x) Serial.println(x)
#define D_STREAM(x) Serial << x << endl
#define D_DELAY(x) delay(x)
#endif

//Pin Mapping of MCU (Wemos D1)
#define ldrPin A0
#define aBtnPin D1
#define bBtnPin D2
#define xBtnPin D3
#define yBtnPin 20 //Tempoarily undefined
#define rBtnPin D8
#define lBtnPin D7
#define dBtnPin D6
#define uBtnPin 20 //Temporarily undefined
#define cPadXPin D0
#define cPadYPin D5

//Credentials for WiFi network Firebase Database
#define FIREBASE_HOST "spcatcher-af3f6.firebaseio.com"
#define FIREBASE_AUTH "lYzKEQ5yAzfgEfS6mMucajbdj2rDhZy2QEJiPSrt"
#define WIFI_SSID "Ding M"
#define WIFI_PASSWORD "gameover"

//Function prototypes, requirement in Arduino for custom types
//State strToState(String str);

#define BLACKOUT_VAL 180
#define SHINY_DELAY_MAX 14100
#define SHINY_DELAY_MIN 4000

String gameVer = "sm";
int pballCount = 50;
int pCount = 0;
int eCount = 0;
int tCount = 0;
State state = IDLE;
Mode mode = FULLY_AUTO;
int update = 0;

void setup()
{
    //Establish serial connection for debugging
    D_BEGIN(9600);

    //Establish WiFi connection
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    D_PRINT("connecting");
    //while (WiFi.status() != WL_CONNECTED)
    //{
    //D_PRINT(".");
    //D_DELAY(500);
    //}
    D_PRINTLN();
    D_PRINT("connected: ");
    //D_PRINTLN(WiFi.localIP());

    //Establish connection to Firebase database
    //Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

    //Initialise pin types
    pinMode(ldrPin, INPUT);
    pinMode(aBtnPin, OUTPUT);
    pinMode(bBtnPin, OUTPUT);
    pinMode(xBtnPin, OUTPUT);
    pinMode(lBtnPin, OUTPUT);
    pinMode(rBtnPin, OUTPUT);
    pinMode(dBtnPin, OUTPUT);
    pinMode(cPadXPin, OUTPUT);
    pinMode(cPadYPin, OUTPUT);

    btnInit();
    spCatcherInit();
}

char btnInput;
int btnDelayInput;

/* void loop(){
    btnInput = 0;
    btnDelayInput = 0;
    Serial.flush();
    while (!Serial.available())
    {
    }
    btnInput = Serial.read();
    btnDelayInput = Serial.parseInt();
    btnPress(btnInput, btnDelayInput);
} */

void loop()
{
    static long startWalk;

    if (state != IDLE && state != ERROR && (tCount <= pCount) && pballCount != 0) //The state to start when button pressed
    {
        D_PRINTLN("SPCatcher Status: Active");
        D_STREAM("LDR Value: " << analogRead(ldrPin));

        if (analogRead(ldrPin) <= BLACKOUT_VAL)
        {
            D_PRINTLN("SPCatcher Status: Pokemon Found");
            long startTime = millis();
            while (analogRead(ldrPin) <= BLACKOUT_VAL){
                delay(0);
            };
            long duration = millis() - startTime;
            D_STREAM("Duration Value: " << duration);

            if (duration > SHINY_DELAY_MAX) //Possible Shiny Pokemon Encountered
            {
                //if((duration > 11800 && duration < 13200) || (duration > 15000)){
                D_PRINTLN("Pokemon Type: Is shiny");
                if (mode == FULLY_AUTO)
                {
                    delay(1000);
                    Firebase.setString("State", "CATCHING");
                    autoCatch(gameVer);
                }
                else if (mode == SEARCH_ONLY)
                {
                    Firebase.setString("State", "IDLE");
                }
                startWalk = millis();
            }

            else if (duration > SHINY_DELAY_MIN && duration < SHINY_DELAY_MAX)
            {
                D_PRINTLN("Pokemon Type: Not shiny");
                escape(gameVer);
                startWalk = millis();
            }
        }

        else
        {
            long durationWalk = millis() - startWalk;
            walkCycle(gameVer);

            if (durationWalk > 90000) //Check if no encounters for 90 seconds
            {
                state = ERROR;
                D_PRINTLN("ERROR: Player is walking for too long");
            }
        }
    }

    else if(state == ERROR){
        D_PRINTLN("SPCatcher Status: ERROR. Needs user input to rectify.");
        Serial.flush();
        while (!Serial.available())
        {
            delay(0);
        }
        String input = Serial.readString();
        state = strToState(input);
        startWalk = millis();
    }

    else //SPCatcher in IDLE mode
    {
        //Firebase.setString("State", "IDLE");
        D_PRINTLN("SPCatcher Status: Idle.");
        Serial.flush();
        while (!Serial.available())
        {
            delay(0);
        }
        String input = Serial.readString();
        state = strToState(input);
        startWalk = millis();
    }

    if (update == true)
    {
        //mode = Firebase.getString("State");
        //state = Firebase.getString("State");
        //tCount = Firebase.getInt("TargetCount");
    }

    //state = Firebase.getString("State");
    //update = Firebase.getBool("Update");
}

//Physical key press of the 3DS buttons
void btnPress(char btn, int pressDelay)
{
    int btnPin = 0;
    int cPadDir = 511;
    switch (btn)
    {
    case 'a':
        btnPin = aBtnPin;
        break;
    case 'b':
        btnPin = bBtnPin;
        break;
    case 'x':
        btnPin = xBtnPin;
        break;
    case 'y':
        //btnPin = yBtnPin;
        break;
    case 'l':
        btnPin = lBtnPin;
        break;
    case 'r':
        btnPin = rBtnPin;
        break;
    case 'u':
        btnPin = uBtnPin;
        break;
    case 'd':
        btnPin = dBtnPin;
        break;
    case 'w':
        btnPin = cPadXPin;
        cPadDir = 1023;
        break;
    case 'e':
        btnPin = cPadXPin;
        cPadDir = 0;
        break;
    case 'n':
        btnPin = cPadYPin;
        cPadDir = 1023;
        break;
    case 's':
        btnPin = cPadYPin;
        cPadDir = 0;
        break;
    }

    if ((btnPin != cPadXPin) && (btnPin != cPadYPin))
    {
        D_STREAM("Digital button " << btn << " is pressed");
        digitalWrite(btnPin, LOW);
        delay(pressDelay);
        digitalWrite(btnPin, HIGH);
    }
    else
    {
        D_STREAM("Analog Circle Pad direction: " << btn);
        analogWrite(btnPin, cPadDir);
        delay(pressDelay);
        analogWrite(btnPin, 511);
    }
}

//Parses the input commands from the buttonsequence.h
void seqDecoderJSON(String gameVer, String seqName)
{
    JsonObject &gameVerSeq = root[gameVer];
    JsonObject &seq = gameVerSeq[seqName];
    JsonArray &seqArray = seq["sequence"].asArray();
    JsonArray &pressDelayArray = seq["press_delay"].asArray();
    JsonArray &delayArray = seq["delay"].asArray();

    for (int x = 0; x < seqArray.size(); x++)
    {
        const char *btnAsChar = seqArray[x];
        btnPress(*btnAsChar, pressDelayArray[x]);
        delay(delayArray[x]);
    }
}

//The buttons needs to be low
void btnInit()
{
    digitalWrite(aBtnPin, HIGH);
    digitalWrite(bBtnPin, HIGH);
    digitalWrite(xBtnPin, HIGH);
    digitalWrite(lBtnPin, HIGH);
    digitalWrite(rBtnPin, HIGH);
    digitalWrite(dBtnPin, HIGH);
    digitalWrite(uBtnPin, HIGH);
    analogWrite(cPadXPin, 511);
    analogWrite(cPadYPin, 511);
}

void spCatcherInit()
{
    //gameVer = firebase.getString("GameVer");
    //pballCount = firebase.getInt("BallCount");
}

//Comamands to control movements in the 3DS
void walkCycle(String gameVer)
{
    static int dir = 0;
    if (analogRead(ldrPin) > BLACKOUT_VAL)
    {
        D_PRINTLN("SPCatcher Status: Searching");
        if(dir == 0){
            seqDecoderJSON(gameVer, "seq_walk_l");
            dir = 1;
        }
        else{
            seqDecoderJSON(gameVer, "seq_walk_r");
            dir = 0;
        }
        
    }
}

void escape(String gameVer)
{
    D_PRINTLN("SPCatcher Status: Escaping..");
    delay(500);
    seqDecoderJSON(gameVer, "seq_escape");
    delay(500);
}

void autoCatch(String gameVer)
{
    static int loopCatch = 0;

    if (pballCount > 0)
    { //If Pokeball count reaches '0', notify user
        D_STREAM("SPCatcher Status: Catching Pokemon. Attempt: " << loopCatch);

        if (loopCatch == 0)
        {
            D_PRINTLN("SPCatcher Status: Weakening Pokemon");
            seqDecoderJSON(gameVer, "seq_attack");
            delay(8000);
            seqDecoderJSON(gameVer, "seq_try_catch");
            pballCount--;
        }
        else if (loopCatch >= 10){
            D_PRINTLN("ERROR: Too many failed attempts!");
            state = ERROR;
        }
        else
        {
            seqDecoderJSON(gameVer, "seq_loop_catch");
            pballCount--;
        }

        delay(28000);

        if (analogRead(ldrPin) <= BLACKOUT_VAL)
        {
            D_PRINTLN("SPCatcher Status: Pokemon caught!");
            delay(1000);
            loopCatch = 0;
            pCount++;
            seqDecoderJSON(gameVer, "seq_caught");
        }

        else
        {
            D_PRINTLN("SPCatcher Status: Catch failed...");
            delay(2000);
            loopCatch++;
            autoCatch(gameVer);
        }
    }
}

void saveGame(String gameVer)
{
    D_PRINTLN("SPCatcher Status: Saving Game");
    seqDecoderJSON(gameVer, "seq_save_game");
}

void functionalTest(char select)
{
    switch (select)
    {
    case 'b':
        //input = 0;
        //Serial.flush();
        //while (!Serial.available())
        //{
        //}
        //input = Serial.read();
        //btnPress(input, 1500);
        //delay(2000);
        break;
    case 'w':
        walkCycle(gameVer);
        break;
    case 'e':
        escape(gameVer);
        break;
    case 'c':
        autoCatch(gameVer);
        break;
    case 's':
        saveGame(gameVer);
        break;
    case 'r':
        D_STREAM("Current reading: " << analogRead(ldrPin));
    }
}