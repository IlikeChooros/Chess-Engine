#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <filesystem>

#include "manager.h"
#include "input_handle.h"


namespace ui
{
    void runWindow(chess::Board& board, int argc, char** argv);
}