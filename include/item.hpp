#ifndef ITEM_HPP
#define ITEM_HPP

#include <stdio.h>
#include <string>
class Item
{
public:
Item(); //default constructor
Item( std::string name, std::string description, int healAmount, int manaAmount,int quantity); //constructor with params
std::string showDescription() const;
std::string showName() const;
int getHealAmount() const;
int getManaAmount() const;
int getQuantity() const;
void addToQuantity(int amount);
void subFromQuantity();


private:
std::string name;
std::string description;
int healAmount, manaAmount, quantity;

};






#endif