#ifndef SPOTIFYCONTROLLER_SPOTIFYPLAYER_H
#define SPOTIFYCONTROLLER_SPOTIFYPLAYER_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SpotifySongInfo.h>
#include <base64.h>

class SpotifyPlayer {
public:
    bool Playing;
    long NextRefresh;
    SpotifySongInfo SongInfo;
    bool hasAccessToken() {
        if(this->AcccessToken == "") return false;
        return true;
    }
    void requestAccessToken() {
        this->https.begin(*this->client,"https://accounts.spotify.com/api/token");
        String auth = "Basic " + base64::encode(String(this->ClientId) + ":" + String(this->ClientSecret));
        this->https.addHeader("Authorization",auth);
        this->https.addHeader("Content-Type","application/x-www-form-urlencoded");
        String requestBody = "grant_type=authorization_code&code=" + this->AccessCode + "&redirect_uri=" + String(this->RedirectUrl);

        int httpResponseCode = this->https.POST(requestBody);
        if (httpResponseCode == HTTP_CODE_OK) {
            String response = this->https.getString();
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);
            this->AcccessToken = String((const char*)doc["access_token"]);
            this->RefreshToken = String((const char*)doc["refresh_token"]);
            long expiresIn = long(doc["expires_in"]);
            expiresIn -= 60;
            expiresIn *= 1000;
            long current = (long)millis();
            this->NextRefresh = current + expiresIn;
        } else {
            Serial.println(this->https.getString());
        }
        this->https.end();
    }
    void setAccessCode(String accessCode) {
        this->AccessCode = accessCode;
    }
    void refreshAccessToken() {
        this->https.begin(*this->client,"https://accounts.spotify.com/api/token");
        String auth = "Basic " + base64::encode(String(this->ClientId) + ":" + String(this->ClientSecret));
        this->https.addHeader("Authorization",auth);
        this->https.addHeader("Content-Type","application/x-www-form-urlencoded");
        String requestBody = "grant_type=refresh_token&refresh_token=" + this->RefreshToken;

        int httpResponseCode = this->https.POST(requestBody);
        if (httpResponseCode == HTTP_CODE_OK) {
            String response = this->https.getString();
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);
            this->AcccessToken = String((const char*)doc["access_token"]);
            this->RefreshToken = String((const char*)doc["refresh_token"]);
            long expiresIn = long(doc["expires_in"]);
            expiresIn -= 60;
            expiresIn *= 1000;
            long current = (long)millis();
            this->NextRefresh = current + expiresIn;
        } else {
            Serial.println(this->https.getString());
        }
        this->https.end();
    }
    void getCurrentlyPlaying() {
        this->https.begin(*this->client,"https://api.spotify.com/v1/me/player/currently_playing?market=DE&additional_types=track,episode");
        String auth = "Bearer " + this->AcccessToken;
        this->https.addHeader("Authorization",auth);

        int httpResponseCode = this->https.GET();
        if (httpResponseCode == HTTP_CODE_OK) {
            String response = this->https.getString();
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, response);
            this->SongInfo.artist = String((const char*)doc["item"]["artists"][0]["name"]);
            this->SongInfo.song = String((const char*)doc["item"]["name"]);
            this->SongInfo.progressMs = int(doc["progress_ms"]);
            this->SongInfo.totalMs = int(doc["item"]["duration_ms"]);
            this->Playing = bool(doc["is_playing"]);
        }else{
            Serial.println(this->https.getString());
        }
        this->https.end();
    }
    void playPreviousSong() {
        this->https.begin(*this->client,"https://api.spotify.com/v1/me/player/previous");
        String auth = "Bearer " + this->AcccessToken;
        this->https.addHeader("Authorization",auth);

        int httpResponseCode = this->https.POST("");
        if (httpResponseCode == HTTP_CODE_OK) {
        }else{
            Serial.println(this->https.getString());
        }
        this->https.end();
    }
    void playNextSong() {
        this->https.begin(*this->client,"https://api.spotify.com/v1/me/player/next");
        String auth = "Bearer " + this->AcccessToken;
        this->https.addHeader("Authorization",auth);

        int httpResponseCode = this->https.POST("");
        if (httpResponseCode == HTTP_CODE_OK) {
        }else{
            Serial.println(this->https.getString());
        }
        this->https.end();
    }
    void togglePlayback() {
        if(this->Playing) {
            this->https.begin(*this->client,"https://api.spotify.com/v1/me/player/pause");
            String auth = "Bearer " + this->AcccessToken;
            this->https.addHeader("Authorization",auth);

            int httpResponseCode = this->https.PUT("");
            if (httpResponseCode == HTTP_CODE_OK) {
            }else{
                Serial.println(this->https.getString());
            }
            this->https.end();
        } else {
            this->https.begin(*this->client,"https://api.spotify.com/v1/me/player/play");
            String auth = "Bearer " + this->AcccessToken;
            this->https.addHeader("Authorization",auth);

            int httpResponseCode = this->https.PUT("");
            if (httpResponseCode == HTTP_CODE_OK) {
            }else{
                Serial.println(this->https.getString());
            }
            this->https.end();
        }
    }
    void changeVolume(int volume) {
        this->https.begin(*this->client,"https://api.spotify.com/v1/me/player/volume?volume_percent="+ String(volume));
        String auth = "Bearer " + this->AcccessToken;
        this->https.addHeader("Authorization",auth);

        int httpResponseCode = this->https.PUT("");
        if (httpResponseCode == HTTP_CODE_OK) {
        }else{
            Serial.println(this->https.getString());
        }
        this->https.end();
    }
    SpotifyPlayer(char* clientId, char* clientSecret, char* redirectUrl) {
        this->ClientId = clientId;
        this->ClientSecret = clientSecret;
        this->RedirectUrl = redirectUrl;
        this->client = std::make_unique<BearSSL::WiFiClientSecure>();
        this->client->setInsecure();
        this->Playing = false;
        this->NextRefresh = 0;
    }
private:
    String AcccessToken;
    String RefreshToken;
    String AccessCode;
    char* ClientId;
    char* ClientSecret;
    char* RedirectUrl;
    HTTPClient https;
    std::unique_ptr<BearSSL::WiFiClientSecure> client;
};
#endif //SPOTIFYCONTROLLER_SPOTIFYPLAYER_H
