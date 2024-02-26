#ifndef SPOTIFYCONTROLLER_SPOTIFYSONGINFO_H
#define SPOTIFYCONTROLLER_SPOTIFYSONGINFO_H
#include <Arduino.h>
class SpotifySongInfo {
    public:
        String artist;
        String song;
        int totalMs;
        int progressMs;
        SpotifySongInfo() {
            this->artist = "";
            this->song = "";
            this->totalMs = 0;
            this->progressMs = 0;
        }
};
#endif //SPOTIFYCONTROLLER_SPOTIFYSONGINFO_H
