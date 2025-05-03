#pragma once

#include "namespaces.hpp"

#include <cengine/engine.h>

UI_NAMESPACE_BEGIN

class GameManager
{
public:
    GameManager();

    void init(chess::Engine& engine);
    bool makeMove(const std::string& move);

private:
    chess::Engine& m_engine;
};


UI_NAMESPACE_END