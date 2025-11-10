#ifndef SOUND_MANAGER_HPP
#define SOUND_MANAGER_HPP

#include <SFML/Audio.hpp>
#include <string>
#include <map>
#include <memory>

class SoundManager
{
private:

    std::map<std::string, sf::Sound> sounds;
    std::map<std::string, sf::SoundBuffer> buffers; 

public:

void loadSound(const std::string& effectname, const std::string& fileName);
void playSound(const std::string& effectname);
void stopSound(const std::string& effectname);
sf::Sound& getRef(const std::string& sound);


SoundManager();
~SoundManager();



};





#endif