/*

This file declares what map can do without the how. The how is map.cpp, hpp serves as a sorta guideline/class

*/

#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Map {
    private:
        std::vector<std::vector<int>> grid; // holds the map data - 0 = empty, 1 = wall
        int width; // map width
        int height; // map height

        float spawnX = -1.0f; // in grid coordinates
        float spawnY = -1.0f;
    
    public:
        Map(); // constructor, sets up the grid when the object is created 
        bool loadFromFile(const std::string& filename); //load map
    
        bool isWall(int x, int y) const; // Check if a given grid cell is a wall, basically collision detection
    
        void renderMiniMap(sf::RenderWindow& window, const sf::View& miniMapView, const sf::Vector2f& playerPos, float playerAngle) const; // add mini map
    
        // Getters for map size
        int getWidth() const { return width; }
        int getHeight() const { return height; }

        // Getters for map spawn
        float getSpawnX() const { return spawnX;}
        float getSpawnY() const { return spawnY;}
        
    };
