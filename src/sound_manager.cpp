#include "sound_manager.hpp"
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>

void SoundManager::loadSound(const std::string& effectName, const std::string& fileName)
{
    sf::SoundBuffer buffer;
    if(!buffer.loadFromFile(fileName)){
        std::cout << "Failed to load sound: " << fileName << std::endl;
        return;
    }
    buffers[effectName] = std::move(buffer);
    sf::Sound sound;
    sound.setBuffer(buffers[effectName]);
    sounds[effectName] = std::move(sound);
    return;
}

void SoundManager::playSound(const std::string& effectName)
{
    auto sound = sounds.find(effectName);
    if (sound != sounds.end())
        sound->second.play();
    else
        std::cerr << "Sound not found: " << effectName << std::endl;
}

void SoundManager::stopSound(const std::string& effectName)
{
    auto sound = sounds.find(effectName);
    if (sound != sounds.end())
        sound->second.stop();
    else
        std::cerr << "Sound not found: " << effectName << std::endl;
}

sf::Sound& SoundManager::getRef(const std::string& effectName)
{
    return this->sounds.at(effectName);
}

SoundManager::SoundManager() = default;
SoundManager::~SoundManager() = default;