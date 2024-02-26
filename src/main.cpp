#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <button.h>
#include <pages.h>
#include <SpotifyPlayer.h>

#define BTN1 14
Button btn1;
#define BTN2 12
Button btn2;
#define BTN3 13
Button btn3;
#define BTN4 2
Button btn4;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET (-1)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ESP8266WebServer server(80);

#define CLIENT_ID "6fa2249dc1e948509284107a50c177af"
#define CLIENT_SECRET "36a96ff7141c4c2b87d03deeb0530752"
#define REDIRECT_URI "http://192.168.178.48/callback"
SpotifyPlayer player((char*)CLIENT_ID, (char*)CLIENT_SECRET, (char*)REDIRECT_URI);

void drawProgressbar(int16_t x, int16_t y, int16_t width, int16_t height, float progress)
{
    progress = progress > 100 ? 100 : progress;
    progress = progress < 0 ? 0 : progress;
    float bar = ((float)(width-1) / 100) * progress;
    display.drawRect(x, y, width, height, WHITE);
    x+=2;
    y+=2;
    height-=4;
    display.fillRect(x, y, (int16_t)bar , (int16_t)height, WHITE);
}

void connectWifi() {
    display.clearDisplay();
    display.println("Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin("FRITZ!Box 7590 GH", "66392583845337291946");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        display.print(".");
        display.display();
    }
    display.println("Connected!");
    display.display();
    display.println("IP Address: ");
    display.display();
    display.print(WiFi.localIP());
    display.display();
    delay(2000);
}

void handleRoot() {
    char page[500];
    sprintf(page,mainPage,CLIENT_ID,REDIRECT_URI);
    server.send(200, "text/html", String(page)+"\r\n");
}

void handleCallback() {
    if(!player.hasAccessToken()){
        if(server.arg("code") == ""){
            char page[500];
            sprintf(page,errorPage,CLIENT_ID,REDIRECT_URI);
            server.send(200, "text/html", String(page));
        } else {
            player.setAccessCode(server.arg("code"));
            player.requestAccessToken();
            server.send(200,"text/html","Spotify setup complete Auth refresh in");
        }
    }else{
        server.send(200,"text/html","Spotify setup complete");
    }
}

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long nextCurrentSongDisplay = 0;
long lastVolumeChecked = 0;
long lastVolume = 0;
bool serverOn = true;

void setup() {
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    connectWifi();

    server.on("/", handleRoot);      //Which routine to handle at root location
    server.on("/callback", handleCallback);      //Which routine to handle at root location
    server.begin();

    btn1.begin(BTN1);
    btn2.begin(BTN2);
    btn3.begin(BTN3);
    btn4.begin(BTN4);
}

void loop() {
    if(!player.hasAccessToken()) {
        server.handleClient();
    } else {
        if(serverOn) {
            serverOn = false;
            server.close();
        }

        if((long)millis() > nextCurrentSongDisplay) {
            player.getCurrentlyPlaying();
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println(player.SongInfo.artist);
            display.println(player.SongInfo.song);
            float progress = (float)player.SongInfo.progressMs / (float)player.SongInfo.totalMs;
            progress *= 100;
            drawProgressbar(14, 50, 100, 10, progress);
            display.display();
            nextCurrentSongDisplay = (long)millis() + 1000;
        }

        if((long)millis() > player.NextRefresh) {
            player.refreshAccessToken();
        }

        if((long)millis() > lastVolumeChecked) {
            int analogValue = analogRead(A0);
            float analogValuePercentile = (float)analogValue / 1024;
            float rawValue = analogValuePercentile * 100;
            int percent = 100 - (int)rawValue;

            if(percent < 0) percent = 0;
            if(percent > 100) percent = 100;

            if(percent + 1 < lastVolume || percent - 1 > lastVolume) {
                lastVolume = percent;
                player.changeVolume(lastVolume);
            }

            lastVolumeChecked = (long)millis() + 1000;
        }

        if(btn1.debounce()) {
            player.playPreviousSong();
        }
        if(btn2.debounce()) {
            player.togglePlayback();
        }
        if(btn3.debounce()) {
            player.playNextSong();
        }
        if(btn4.debounce()) {
            Serial.println("Button 4 Pressed");
        }
    }
}