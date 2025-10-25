#include "item.hpp"
#include <string>
Item::Item(std::string description, std::string name, int healAmount, int manaAmount,int quantity) : description(description), name(name), healAmount(healAmount), manaAmount(manaAmount), quantity(quantity){} //params
Item::Item() : description("None"), name("Empty Slot"), healAmount(0), manaAmount(0), quantity(0){} //default
std::string Item::showDescription(){
    return description;
}
std::string Item::showName(){
    return name;
}
int Item::getHealAmount(){
    return healAmount;
}
int Item::getManaAmount(){
    return manaAmount;
}
int Item::getQuantity(){
    return quantity;
}
void Item::addToQuantity(int amount){
    if (this->quantity < 100){
        this->quantity += amount;
    }
}