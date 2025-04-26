// File: src/GameObject.cpp
#include "GameObject.h"

void GameObject::setProperty(const std::string& key, const PropertyValue& value) {
    properties[key] = value;
}