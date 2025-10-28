#include <SFML/Graphics.hpp>
#include <map>
#include <string>

#include "texture_manager.hpp"

void TextureManager::loadTexture(const std::string& name, const std::string& filename)
{
    //load textures
    sf::Texture tex;
    tex.loadFromFile(filename);
    //add to texture list
    this->textures[name] = std::move(tex);

    return;
}

sf::Texture& TextureManager::getRef(const std::string& texture)
{
    return this->textures.at(texture);
}