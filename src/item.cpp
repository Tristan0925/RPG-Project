#include "item.hpp"
#include <string>
Item::Item(std::string name,std::string description, int healAmount, int manaAmount,int quantity) : name(name), description(description), healAmount(healAmount), manaAmount(manaAmount), quantity(quantity){} //params
Item::Item() :  name("Empty Slot"), description("None"),healAmount(0), manaAmount(0), quantity(0){} //default
std::string Item::showDescription() const{
    return description;
}
std::string Item::showName() const{
    return name;
}
int Item::getHealAmount() const{
    return healAmount;
}
int Item::getManaAmount() const{
    return manaAmount;
}
int Item::getQuantity() const{
    return quantity;
}
void Item::addToQuantity(int amount){
    if (this->quantity < 100){
        this->quantity += amount;
    }
}
void Item::subFromQuantity(){
    this->quantity -= 1;
}

//add use item method later...