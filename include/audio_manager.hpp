#ifndef AUDIO_MANAGER_HPP
#define AUDIO_MANAGER_HPP

#include <SFML/Audio.hpp>
#include <string>
#include <map>

class AudioManager
{
private:

    std::map<std::string, sf::Sound> Sounds;
    std::map<std::string, sf::Music> Tracks;

public:

void loadSound();
void loadSong();
void changeSong();






};





#endif