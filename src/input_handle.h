#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

#include "manager.h"

void handleInput(chess::Manager* manager, sf::Event& event, sf::RenderWindow* window);
void getBoardSize(int& size, int& offset_x, sf::RenderWindow* window);