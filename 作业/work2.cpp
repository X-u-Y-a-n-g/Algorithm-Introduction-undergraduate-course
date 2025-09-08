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
    add_edge(graph, { "A2", "L1" }, { "B2", "L2" }, transfer_time);   // A2站：L1和L2换乘
    add_edge(graph, { "A19", "L1" }, { "B3", "L2" }, transfer_time); // A10站：L1和L3换乘
    add_edge(graph, { "A15", "L1" }, { "E15", "L5" }, transfer_time); // A15站：L1和L5换乘
    add_edge(graph, { "C12", "L3" }, { "D12", "L4" }, transfer_time); // C12站：L3和L4换乘
    add_edge(graph, { "B8", "L2" }, { "D8", "L4" }, transfer_time);   // B8站：L2和L4换乘
    add_edge(graph, { "E18", "L5" }, { "C18", "L3" }, transfer_time); // E18站：L5和L3换乘
}

vector<Node> dijkstra(const Graph& graph, Node start, Node end) {
    unordered_map<Node, int, NodeHash> dist;
    unordered_map<Node, Node, NodeHash> prev;

    for (const auto& pair : graph)
        dist[pair.first] = numeric_limits<int>::max();

    auto cmp = [&dist](Node a, Node b) { return dist[a] > dist[b]; };
    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);

    if (dist.find(start) == dist.end() || dist.find(end) == dist.end()) return {};

    dist[start] = 0;
    pq.push(start);

    while (!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        if (cur == end) break;

        for (auto& [nei, cost] : graph.at(cur)) {
            int alt = dist[cur] + cost;
            if (alt < dist[nei]) {
                dist[nei] = alt;
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
    return path;
}

// 修改 dijkstra 函数，添加权重平衡
vector<Node> balanced_dijkstra(const Graph& graph, Node start, Node end, double alpha = 0.5) {
    struct NodeInfo {
        int time;           // 总时间
        int transfers;      // 换乘次数
        Node prev;          // 前驱节点
        string curr_line;   // 当前所在线路
    };
    
    unordered_map<Node, NodeInfo, NodeHash> info;
    
    // 初始化
    for (const auto& pair : graph) {
        info[pair.first] = {numeric_limits<int>::max(), 0, Node(), ""};
    }
    
    auto cmp = [&info, alpha](const Node& a, const Node& b) {
        double score_a = alpha * info[a].time + (1-alpha) * info[a].transfers * 10;
        double score_b = alpha * info[b].time + (1-alpha) * info[b].transfers * 10;
        return score_a > score_b;
    };
    
    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);
    
    if (info.find(start) == info.end() || info.find(end) == info.end()) 
        return {};

    info[start] = {0, 0, start, start.line};
    pq.push(start);

    while (!pq.empty()) {
        Node cur = pq.top(); pq.pop();
        if (cur == end) break;

        for (auto& [nei, time] : graph.at(cur)) {
            // 计算换乘次数
            int new_transfers = info[cur].transfers;
            if (nei.line != info[cur].curr_line) {
                new_transfers++;
            }
            
            // 计算总时间
            int new_time = info[cur].time + time;
            
            // 计算综合得分
            double new_score = alpha * new_time + (1-alpha) * new_transfers * 10;
            double old_score = alpha * info[nei].time + (1-alpha) * info[nei].transfers * 10;
            
            if (new_score < old_score) {
                info[nei] = {new_time, new_transfers, cur, nei.line};
                pq.push(nei);
            }
        }
    }

    // 构建路径
    vector<Node> path;
    if (info[end].time == numeric_limits<int>::max()) return path;
    
    for (Node at = end; !(at == start); at = info[at].prev) {
        path.push_back(at);
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    
    // 输出路径信息
    cout << "总时间: " << info[end].time << " 分钟\n";
    cout << "换乘次数: " << info[end].transfers << " 次\n";
    
    return path;
}

int main() {
    system("chcp 65001");
    Graph graph;
    build_graph(graph);

    // 测试不同权重系数
    vector<double> alphas = {0.3, 0.7};
    
    Node start = {"A1", "L1"};
    Node end = {"A20", "L1"};

    for (double alpha : alphas) {
        cout << "\n使用权重系数 α = " << alpha << "\n";
        cout << "测试路径: " << start.name << "@" << start.line 
             << " -> " << end.name << "@" << end.line << "\n";
        
        auto path = balanced_dijkstra(graph, start, end, alpha);

        if (path.empty()) {
            cout << "未找到可达路径！\n";
        } else {
            cout << "平衡路径：\n";
            for (auto& node : path)
                cout << node.name << "@" << node.line << " -> ";
            cout << "END\n";
        }
    }

    return 0;
}