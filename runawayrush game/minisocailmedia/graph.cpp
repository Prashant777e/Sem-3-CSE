#include "graph.h"
#include "database.h"
#include <algorithm>

Graph::Graph() {}

void Graph::addNode(int userId) {
    if (adjacencyList.find(userId) == adjacencyList.end()) {
        adjacencyList[userId] = std::unordered_set<int>();
    }
}

void Graph::addEdge(int userId1, int userId2) {
    addNode(userId1);
    addNode(userId2);
    adjacencyList[userId1].insert(userId2);
    adjacencyList[userId2].insert(userId1);
}

void Graph::removeEdge(int userId1, int userId2) {
    if (adjacencyList.count(userId1)) {
        adjacencyList[userId1].erase(userId2);
    }
    if (adjacencyList.count(userId2)) {
        adjacencyList[userId2].erase(userId1);
    }
}

void Graph::removeNode(int userId) {
    if (adjacencyList.count(userId)) {
        // Remove all edges to this node
        for (int neighbor : adjacencyList[userId]) {
            adjacencyList[neighbor].erase(userId);
        }
        adjacencyList.erase(userId);
    }
}

bool Graph::hasEdge(int userId1, int userId2) const {
    auto it = adjacencyList.find(userId1);
    if (it != adjacencyList.end()) {
        return it->second.count(userId2) > 0;
    }
    return false;
}

std::vector<int> Graph::getNeighbors(int userId) const {
    std::vector<int> neighbors;
    auto it = adjacencyList.find(userId);
    if (it != adjacencyList.end()) {
        neighbors.assign(it->second.begin(), it->second.end());
    }
    return neighbors;
}

int Graph::getConnectionCount(int userId) const {
    auto it = adjacencyList.find(userId);
    return it != adjacencyList.end() ? it->second.size() : 0;
}

// Find mutual friends between two users
std::vector<int> Graph::getMutualFriends(int userId1, int userId2) const {
    std::vector<int> mutual;

    auto it1 = adjacencyList.find(userId1);
    auto it2 = adjacencyList.find(userId2);

    if (it1 != adjacencyList.end() && it2 != adjacencyList.end()) {
        for (int friend1 : it1->second) {
            if (it2->second.count(friend1)) {
                mutual.push_back(friend1);
            }
        }
    }
    return mutual;
}

// Friend suggestions based on mutual friends (friends of friends)
std::vector<int> Graph::getSuggestedFriends(int userId) const {
    std::unordered_map<int, int> suggestionScore;

    auto it = adjacencyList.find(userId);
    if (it == adjacencyList.end()) {
        return {};
    }

    // For each friend
    for (int friendId : it->second) {
        auto friendIt = adjacencyList.find(friendId);
        if (friendIt != adjacencyList.end()) {
            // For each friend of friend
            for (int fof : friendIt->second) {
                // Don't suggest yourself or existing friends
                if (fof != userId && !it->second.count(fof)) {
                    suggestionScore[fof]++;
                }
            }
        }
    }

    // Sort by score (most mutual friends first)
    std::vector<std::pair<int, int>> scorePairs;
    for (auto& p : suggestionScore) {
        scorePairs.push_back(p);
    }

    std::sort(scorePairs.begin(), scorePairs.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<int> suggestions;
    for (auto& p : scorePairs) {
        suggestions.push_back(p.first);
    }
    return suggestions;
}

// BFS to find shortest path
std::vector<int> Graph::getShortestPath(int from, int to) const {
    if (from == to) return {from};
    if (!adjacencyList.count(from) || !adjacencyList.count(to)) return {};

    std::unordered_map<int, int> parent;
    std::unordered_set<int> visited;
    std::queue<int> q;

    q.push(from);
    visited.insert(from);
    parent[from] = -1;

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        if (current == to) {
            // Reconstruct path
            std::vector<int> path;
            int node = to;
            while (node != -1) {
                path.push_back(node);
                node = parent[node];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        auto it = adjacencyList.find(current);
        if (it != adjacencyList.end()) {
            for (int neighbor : it->second) {
                if (!visited.count(neighbor)) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }
    }

    return {}; // No path found
}

void Graph::clear() {
    adjacencyList.clear();
}

void Graph::rebuildFromDatabase() {
    clear();

    // Add all users as nodes
    QVector<User*> users = Database::instance().getAllUsers();
    for (User* user : users) {
        addNode(user->getUserId());
        delete user;
    }

    // Add all friendships as edges
    for (auto& pair : adjacencyList) {
        int userId = pair.first;
        QVector<int> friends = Database::instance().getFriends(userId);
        for (int friendId : friends) {
            if (userId < friendId) { // Add edge only once
                addEdge(userId, friendId);
            }
        }
    }
}
