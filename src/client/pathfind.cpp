#include "pathfind.h"

#include <queue>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <map>
#include "mapnode.h"
#include "nodedef.h"


// --- Performance Logging Utilities ---
struct PerfStats {
    long long call_count = 0;
    std::chrono::nanoseconds total_time{0};
};

static std::map<std::string, PerfStats> perf_data;

class PerfTimer {
public:
    PerfTimer(const std::string& name)
        : function_name(name), start_time(std::chrono::high_resolution_clock::now()) {}

    ~PerfTimer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        perf_data[function_name].call_count++;
        perf_data[function_name].total_time += duration;
    }

private:
    std::string function_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};

#define PERF_TIMER(name) PerfTimer timer(name)


// Hash and equality for v3s16
struct v3s16_hash {
    std::size_t operator()(const v3s16& v) const {
        return std::hash<int>()(v.X) ^ (std::hash<int>()(v.Y) << 1) ^ (std::hash<int>()(v.Z) << 2);
    }
};

struct v3s16_equal {
    bool operator()(const v3s16& a, const v3s16& b) const {
        return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
    }
};

static v3f to_v3f(const v3s16 &pos) {
    return v3f(pos.X, pos.Y, pos.Z);
}

static v3s16 to_v3s16(const v3f &pos) {
    return v3s16(static_cast<s16>(std::round(pos.X)), static_cast<s16>(std::round(pos.Y)), static_cast<s16>(std::round(pos.Z)));
}

// Caches
static std::unordered_map<std::string, double> dig_time_cache;

static s32 get_g_cost(const v3f &a, const v3f &b) {
    PERF_TIMER("get_g_cost");
    return static_cast<s32>((a - b).getLength() * 10);
}

double check_tool_dig_time(const ToolCapabilities &toolcaps, const ItemGroupList &node_groups, double old_best_time) {
    PERF_TIMER("check_tool_dig_time");
    double best_time = old_best_time;

    for (const auto &groupcap_pair : toolcaps.groupcaps) {
        const std::string &group = groupcap_pair.first;
        const ToolGroupCap &groupcap = groupcap_pair.second;

        auto it = node_groups.find(group);
        if (it != node_groups.end()) {
            int level = it->second;
            auto level_it = std::find_if(groupcap.times.begin(), groupcap.times.end(),
                [level](const std::pair<int, float> &p) { return p.first == level; });

            if (level_it != groupcap.times.end() && level_it->second < best_time)
                best_time = level_it->second;
        }
    }

    return best_time;
}

double get_best_dig_time(Client *client, const ItemGroupList &groups) {
    PERF_TIMER("get_best_dig_time");
    std::ostringstream oss;
    for (const auto &g : groups)
        oss << g.first << ":" << g.second << ",";
    std::string key = oss.str();

    auto it = dig_time_cache.find(key);
    if (it != dig_time_cache.end())
        return it->second;

    double best_time = std::numeric_limits<double>::infinity();

    InventoryLocation loc;
    loc.setCurrentPlayer();
    Inventory *inv = client->getInventory(loc);
    if (!inv) return best_time;

    InventoryList *main = inv->getList("main");
    if (!main) return best_time;

    for (u32 i = 0; i < main->getSize(); i++) {
        const ItemStack &stack = main->getItem(i);
        if (stack.empty()) continue;

        const ToolCapabilities &toolcaps = stack.getToolCapabilities(client->idef());
        best_time = check_tool_dig_time(toolcaps, groups, best_time);
    }

    dig_time_cache[key] = best_time;
    return best_time;
}

static std::unordered_map<std::string, s32> node_cost_cache;

static s32 get_node_cost(const v3f &pos, Client *client, const NodeDefManager *ndef) {
    PERF_TIMER("get_node_cost");
    v3s16 p = to_v3s16(pos);

    bool pos_ok;
    MapNode node = client->CSMGetNode(p, &pos_ok);
    const ContentFeatures &node_def = ndef->get(node);

    // Cache based on node_def name
    auto it = node_cost_cache.find(node_def.name);
    if (it != node_cost_cache.end())
        return it->second;

    s32 cost;
    if (!pos_ok) {
        cost = std::numeric_limits<s32>::max();
    } else if (node_def.name == "air") {
        cost = 0;
    } else {
        cost = static_cast<s32>(get_best_dig_time(client, node_def.groups) * 200);
    }

    node_cost_cache[node_def.name] = cost;
    return cost;
}

static bool get_node_walkable(const v3f &pos, Client *client, const NodeDefManager *ndef) {
    PERF_TIMER("get_node_walkable");
    v3s16 p = to_v3s16(pos);

    bool pos_ok;
    MapNode node = client->CSMGetNode(p, &pos_ok);
    const ContentFeatures &node_def = ndef->get(node);
    return node_def.walkable;
}


static s32 get_path_segment_cost(const v3f &current_pos, const v3f &from_pos, Client *client, const NodeDefManager *ndef) {
    PERF_TIMER("get_path_segment_cost");
    s32 cost = 0;

    v3s16 from = to_v3s16(from_pos);
    v3s16 to = to_v3s16(current_pos);
    v3s16 delta = to - from;

    // Cost of current node and head clearance at current node
    s32 base_cost = get_node_cost(current_pos, client, ndef);
    if (base_cost == std::numeric_limits<s32>::max()) return base_cost;
    cost += base_cost;

    s32 head_cost = get_node_cost(current_pos + v3f(0, 1, 0), client, ndef);
    if (head_cost == std::numeric_limits<s32>::max()) return head_cost;
    cost += head_cost;

    // Diagonal movement: check side blocks that would be intersected
    if (delta.X != 0 && delta.Z != 0) {
        // Blocks at current Y level
        s32 side1_cost = get_node_cost(from_pos + v3f(delta.X, 0, 0), client, ndef);
        if (side1_cost == std::numeric_limits<s32>::max()) return side1_cost;
        cost += side1_cost;

        s32 side2_cost = get_node_cost(from_pos + v3f(0, 0, delta.Z), client, ndef);
        if (side2_cost == std::numeric_limits<s32>::max()) return side2_cost;
        cost += side2_cost;

        // Blocks at Y+1 (head clearance)
        s32 side1_head_cost = get_node_cost(from_pos + v3f(delta.X, 1, 0), client, ndef);
        if (side1_head_cost == std::numeric_limits<s32>::max()) return side1_head_cost;
        cost += side1_head_cost;

        s32 side2_head_cost = get_node_cost(from_pos + v3f(0, 1, delta.Z), client, ndef);
        if (side2_head_cost == std::numeric_limits<s32>::max()) return side2_head_cost;
        cost += side2_head_cost;
    }

    // Upward movement: check blocks stepped over and additional head clearance
    if (delta.Y > 0) {
        // Check blocks from `from_pos` to `current_pos` along the Y axis
        for (int i = 1; i < delta.Y + 1; ++i) { // +1 for the current_pos's Y level
             v3f intermediate_pos_y = from_pos + v3f(0, i, 0);
             s32 c = get_node_cost(intermediate_pos_y, client, ndef);
             if (c == std::numeric_limits<s32>::max()) return c;
             cost += c;
        }
        // Additional head clearance for upward movement (block 2 units above current_pos)
        s32 c = get_node_cost(current_pos + v3f(0, 2, 0), client, ndef);
        if (c == std::numeric_limits<s32>::max()) return c;
        cost += c;

    }
    
    // Downward movement: additional head clearance for destination
    if (delta.Y < 0) {
        s32 c = get_node_cost(current_pos + v3f(0, 2, 0), client, ndef);
        if (c == std::numeric_limits<s32>::max()) return c;
        cost += c;
    }

    v3f below_current = current_pos - v3f(0, 1, 0);
    bool walkable_below = get_node_walkable(below_current, client, ndef);
    if (!walkable_below) {
        cost += 350; // Penalty for having to place a block
    }

    return cost;
}

PathNode::PathNode(const v3f &pos, const v3f &target, Client *client, const NodeDefManager *ndef, PathNode* owner)
    : position(pos), target_position(target), parent(owner)
{
    PERF_TIMER("PathNode::PathNode");
    
    s32 current_node_a_cost = get_path_segment_cost(pos, owner ? owner->position : pos, client, ndef);

    h_cost = get_g_cost(pos, target);

    if (owner) {
        g_cost = owner->g_cost + get_g_cost(pos, owner->position) + current_node_a_cost;
    } else {
        g_cost = current_node_a_cost;
    }
    
    f_cost = g_cost + h_cost;
}

struct NodeCompare {
    bool operator()(const PathNode* a, const PathNode* b) const {
        return a->f_cost > b->f_cost;
    }
};

static std::vector<v3s16> get_neighbors(const v3s16 &pos) {
    PERF_TIMER("get_neighbors");
    std::vector<v3s16> neighbors;
    for (s16 dx = -1; dx <= 1; ++dx)
        for (s16 dy = -1; dy <= 1; ++dy)
            for (s16 dz = -1; dz <= 1; ++dz)
                if (!(dx == 0 && dy == 0 && dz == 0))
                    neighbors.emplace_back(pos.X + dx, pos.Y + dy, pos.Z + dz);
    return neighbors;
}

Pathfind::Pathfind() {}

std::vector<PathNode> Pathfind::get_path(v3f start, v3f end, Client *client, const NodeDefManager *ndef, int max_depth, bool debug) {
    auto pathfind_start_time = std::chrono::high_resolution_clock::now();
    perf_data.clear();
    dig_time_cache.clear();
    node_cost_cache.clear();


    v3s16 start_pos = to_v3s16(start);
    v3s16 end_pos = to_v3s16(end);

    std::priority_queue<PathNode*, std::vector<PathNode*>, NodeCompare> open_set;
    std::unordered_map<v3s16, PathNode*, v3s16_hash, v3s16_equal> all_nodes;

    std::set<v3s16> closed_set;

    PathNode* start_node = new PathNode(start, end, client, ndef, nullptr);
    open_set.push(start_node);
    all_nodes[start_pos] = start_node;

    PathNode* best_node = start_node;
    PathNode* result_node = nullptr;
    int depth = 0;
    int nodes_checked = 0;

    while (!open_set.empty() && depth < max_depth) {
        PathNode* current = open_set.top();
        open_set.pop();
        nodes_checked++;

        v3s16 current_pos = to_v3s16(current->position);

        if (closed_set.count(current_pos)) {
            continue;
        }

        closed_set.insert(current_pos);

        if (current->h_cost < best_node->h_cost) {
             best_node = current;
        }


        if (current_pos == end_pos) {
            result_node = current;
            break; // Path found, exit loop
        }

		depth++;

        for (const v3s16 &neighbor_pos : get_neighbors(current_pos)) {
            if (closed_set.count(neighbor_pos))
                continue;

            v3f neighbor_f = to_v3f(neighbor_pos);


            s32 tentative_g_cost_for_neighbor;
            s32 current_to_neighbor_segment_cost = get_g_cost(neighbor_f, current->position);
            s32 neighbor_a_cost = get_path_segment_cost(neighbor_f, current->position, client, ndef);
            
            if (neighbor_a_cost == std::numeric_limits<s32>::max()) {
                continue; 
            }

            tentative_g_cost_for_neighbor = current->g_cost + current_to_neighbor_segment_cost + neighbor_a_cost;
            
            auto it = all_nodes.find(neighbor_pos);
            if (it == all_nodes.end()) {
                PathNode* new_node = new PathNode(neighbor_f, end, client, ndef, current);
                all_nodes[neighbor_pos] = new_node;
                open_set.push(new_node);
            } else {
                PathNode* existing_node = it->second;

                if (tentative_g_cost_for_neighbor < existing_node->g_cost) {
                    existing_node->parent = current;
                    existing_node->g_cost = tentative_g_cost_for_neighbor;
                    existing_node->f_cost = existing_node->g_cost + existing_node->h_cost;

                    open_set.push(existing_node);
                }
            }
        }
    }

    if (!result_node) {
        result_node = best_node;
		if (debug) {
       		errorstream << "Path not found to exact target. Falling back to best node found." << std::endl;
		}
    }

    std::vector<PathNode> path;
    PathNode* current_path_node = result_node;
    while (current_path_node) {
        path.push_back(*current_path_node);
        current_path_node = current_path_node->parent;
    }
    std::reverse(path.begin(), path.end());
	if (debug) {
		// --- Final Logging ---
		auto pathfind_end_time = std::chrono::high_resolution_clock::now();
		auto total_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(pathfind_end_time - pathfind_start_time).count();

		errorstream << "========== Pathfind End ==========" << std::endl;
		errorstream << "Pathfinding from (" << start.X << "," << start.Y << "," << start.Z
			<< ") to (" << end.X << "," << end.Y << "," << end.Z << ")" << std::endl;
		errorstream << "Result: " << (result_node && to_v3s16(result_node->position) == end_pos ? "Full path found" : "Fallback path") << std::endl;
		errorstream << "Total time: " << total_duration_ms << " ms" << std::endl;
		errorstream << "Nodes checked from open set: " << nodes_checked << std::endl;
		errorstream << "Max depth reached (iterations): " << depth << " / " << max_depth << std::endl; // Depth as iteration count
		errorstream << "Path length: " << path.size() << " nodes" << std::endl;
		errorstream << "------------------------------------" << std::endl;
		errorstream << "Function Performance Stats:" << std::endl;
		errorstream << "------------------------------------" << std::endl;
		errorstream << std::fixed << std::setprecision(4);

		for (const auto& pair : perf_data) {
			const std::string& name = pair.first;
			const PerfStats& stats = pair.second;
			if (stats.call_count > 0) {
				double total_ms = std::chrono::duration_cast<std::chrono::microseconds>(stats.total_time).count() / 1000.0;
				double avg_us = std::chrono::duration_cast<std::chrono::nanoseconds>(stats.total_time).count() / (double)stats.call_count / 1000.0;
				errorstream << name << ":" << std::endl;
				errorstream << "    Calls: " << stats.call_count << std::endl;
				errorstream << "    Total Time: " << total_ms << " ms" << std::endl;
				errorstream << "    Avg. Time: " << avg_us << " us/call" << std::endl;
			}
		}
		errorstream << "==================================" << std::endl;
	}
    

    for (auto &[_, node] : all_nodes) {
        delete node;
    }

    return path;
}