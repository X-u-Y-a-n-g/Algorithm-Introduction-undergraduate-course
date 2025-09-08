#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <string>
#include <limits>
#include <algorithm>
#include <tuple>

using namespace std;

struct Node {
    string name;
    string line;

    bool operator==(const Node& other) const {
        return name == other.name && line == other.line;
    }

    bool operator!=(const Node& other) const {
        return !(*this == other);
    }

    bool operator<(const Node& other) const {
        return tie(name, line) < tie(other.name, other.line);
    }
};

struct NodeHash {
    size_t operator()(const Node& n) const {
        return hash<string>()(n.name + "@" + n.line);
    }
};

using Graph = unordered_map<Node, vector<pair<Node, int>>, NodeHash>;

void add_edge(Graph& g, Node a, Node b, int time) {
    g[a]; // Ensure node exists
    g[b];
    g[a].emplace_back(b, time);
    g[b].emplace_back(a, time);
}

void build_graph(Graph& graph, int travel_time = 1, int transfer_time = 3) {
    // Add 5 lines of 20 stations each
    for (int i = 1; i <= 20; ++i) {
        string a = "A" + to_string(i);
        string b = "B" + to_string(i);
        string c = "C" + to_string(i);
        string d = "D" + to_string(i);
        string e = "E" + to_string(i);

        graph[{a, "L1"}];
        graph[{b, "L2"}];
        graph[{c, "L3"}];
        graph[{d, "L4"}];
        graph[{e, "L5"}];

        if (i < 20) {
            add_edge(graph, { a, "L1" }, { "A" + to_string(i + 1), "L1" }, travel_time);
            add_edge(graph, { b, "L2" }, { "B" + to_string(i + 1), "L2" }, travel_time);
            add_edge(graph, { c, "L3" }, { "C" + to_string(i + 1), "L3" }, travel_time);
            add_edge(graph, { d, "L4" }, { "D" + to_string(i + 1), "L4" }, travel_time);
            add_edge(graph, { e, "L5" }, { "E" + to_string(i + 1), "L5" }, travel_time);
        }
    }

    // 添加换乘边
    add_edge(graph, { "A5", "L1" }, { "B5", "L2" }, transfer_time);   // A5站：L1和L2换乘
    add_edge(graph, { "A10", "L1" }, { "C10", "L3" }, transfer_time); // A10站：L1和L3换乘
    add_edge(graph, { "A15", "L1" }, { "E15", "L5" }, transfer_time); // A15站：L1和L5换乘
    add_edge(graph, { "C12", "L3" }, { "D12", "L4" }, transfer_time); // C12站：L3和L4换乘
    add_edge(graph, { "B8", "L2" }, { "D8", "L4" }, transfer_time);   // B8站：L2和L4换乘
    add_edge(graph, { "E18", "L5" }, { "C18", "L3" }, transfer_time); // E18站：L5和L3换乘
}

vector<Node> dijkstra(const Graph& graph, Node start, Node end) {
    unordered_map<Node, int, NodeHash> dist;
    unordered_map<Node, Node, NodeHash> prev;
    unordered_map<Node, int, NodeHash> transfers;  // 记录到达每个节点的换乘次数
    unordered_map<Node, string, NodeHash> curr_line;  // 记录到达每个节点时的当前线路

    for (const auto& pair : graph) {
        dist[pair.first] = numeric_limits<int>::max();
        transfers[pair.first] = 0;
    }

    auto cmp = [&dist](Node a, Node b) { return dist[a] > dist[b]; };
    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);

    if (dist.find(start) == dist.end() || dist.find(end) == dist.end()) return {};

    dist[start] = 0;
    curr_line[start] = start.line;
    pq.push(start);

    while (!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        if (cur == end) break;

        for (auto& [nei, cost] : graph.at(cur)) {
            int alt = dist[cur] + cost;
            int new_transfers = transfers[cur];
            
            // 如果线路发生变化，增加换乘次数
            if (nei.line != curr_line[cur]) {
                new_transfers++;
            }

            if (alt < dist[nei]) {
                dist[nei] = alt;
                transfers[nei] = new_transfers;
                curr_line[nei] = nei.line;
                prev[nei] = cur;
                pq.push(nei);
            }
        }
    }

    vector<Node> path;
    if (prev.find(end) == prev.end()) return path;

    for (Node at = end; at != start; at = prev[at]) path.push_back(at);
    path.push_back(start);
    reverse(path.begin(), path.end());

    // 输出总时间和换乘次数
    cout << "总时间: " << dist[end] << " 分钟\n";
    cout << "换乘次数: " << transfers[end] << " 次\n";

    return path;
}

int main() {
    system("chcp 65001");
    Graph graph;
    build_graph(graph);

    // 测试多个路径
    vector<pair<Node, Node>> test_cases = {
        {{"A1", "L1"}, {"A10", "L1"}},    // 同一线路测试
        {{"A1", "L1"}, {"B5", "L2"}},      // 测试换乘点
        {{"A1", "L1"}, {"E20", "L5"}},     // 原始测试用例
        {{"A1", "L1"}, {"E15", "L5"}}      // 通过A15换乘到L5
    };

    for (const auto& [start, end] : test_cases) {
        cout << "\n测试路径: " << start.name << "@" << start.line 
             << " -> " << end.name << "@" << end.line << "\n";
        
        auto path = dijkstra(graph, start, end);

        if (path.empty()) {
            cout << "未找到可达路径！\n";
        } else {
            cout << "最短路径：\n";
            for (auto& node : path)
                cout << node.name << "@" << node.line << " -> ";
            cout << "END\n";
        }
    }

    return 0;
}