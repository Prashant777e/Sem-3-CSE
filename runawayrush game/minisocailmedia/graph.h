#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

class Graph {
public:
    Graph();

    // Core operations
    void addNode(int userId);
    void addEdge(int userId1, int userId2);
    void removeEdge(int userId1, int userId2);
    void removeNode(int userId);

    // Query operations
    bool hasEdge(int userId1, int userId2) const;
    std::vector<int> getNeighbors(int userId) const;
    int getConnectionCount(int userId) const;

    // Advanced algorithms
    std::vector<int> getMutualFriends(int userId1, int userId2) const;
    std::vector<int> getSuggestedFriends(int userId) const;
    std::vector<int> getShortestPath(int from, int to) const; // BFS

    void clear();
    void rebuildFromDatabase();

private:
    // Adjacency list: userId -> set of friend IDs
    std::unordered_map<int, std::unordered_set<int>> adjacencyList;
};

#endif // GRAPH_H
