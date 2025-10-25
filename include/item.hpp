#ifndef ITEM_HPP
#define ITEM_HPP

#include <stdio.h>
#include <string>
class Item
{
public:
Item(); //default constructor
Item(std::string description, std::string name, int healAmount, int manaAmount,int quantity); //constructor with params
std::string showDescription();
std::string showName();
int getHealAmount();
int getManaAmount();
int getQuantity();
void addToQuantity(int amount);


private:
std::string description;
std::string name;
int healAmount; 
int manaAmount;
int quantity;

};






#endif