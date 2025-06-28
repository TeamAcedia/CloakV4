#pragma once

#include <vector>
#include "irrlichttypes_bloated.h"
#include "client/client.h"

class PathNode {
public:
    v3f position;
    v3f target_position;
    s32 g_cost = 0;
    s32 h_cost = 0;
    s32 f_cost = 0;
	s32 a_cost = 0; // cost based on terrain
    PathNode* parent = nullptr;

    PathNode() = default;
    PathNode(const v3f &pos, const v3f &target, Client *client, const NodeDefManager *ndef, PathNode* owner = nullptr);
};



class Pathfind {
public:
    Pathfind();
	std::vector<PathNode> get_path(v3f start, v3f end, Client *client, const NodeDefManager *ndef, int max_depth = 10000, bool debug = false);
};