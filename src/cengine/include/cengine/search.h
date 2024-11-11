#pragma once

#include <unordered_map>


#include "log.h"
#include "eval.h"
#include "move_gen.h"
#include "hash.h"
#include "cache.h"
#include "transp_table.h"
#include "move_ordering.h"
#include "search_utils.h"


namespace chess
{
    void search(Board* board, GameHistory* gh, SearchCache* sc, SearchLimits* params, SearchResult* sr);
}
