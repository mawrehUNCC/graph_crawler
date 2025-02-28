#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <curl/curl.h>
#include "rapidjson/include/rapidjson/document.h"

using namespace std;
using namespace rapidjson;

// API base URL
const string API_BASE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// Define the Node structure
struct Node {
    string name;
    int depth;
};

// Function to handle cURL response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// URL encoding function
string urlencode(const string& str) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "cURL initialization failed!" << endl;
        return "";
    }

    char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
    string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

// Function to fetch neighbors from the API
vector<string> get_neighbors(const string& node) {
    vector<string> neighbors;
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "cURL initialization failed!" << endl;
        return neighbors;
    }

    string url = API_BASE_URL + urlencode(node); // Encode the URL
    string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set header for JSON response
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "cURL request failed: " << curl_easy_strerror(res) << endl;
        curl_easy_cleanup(curl);
        return neighbors;
    }

    // Check HTTP status code
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        cerr << "HTTP request failed with status code: " << http_code << endl;
        curl_easy_cleanup(curl);
        return neighbors;
    }

    // Print raw response (for debugging purposes)
    cout << "Raw Response:\n" << response << endl;

    curl_easy_cleanup(curl);

    // Parse JSON response
    Document doc;
    doc.Parse(response.c_str());

    if (!doc.IsArray()) {
        cerr << "Invalid JSON response" << endl;
        return neighbors;
    }

    for (auto& v : doc.GetArray()) {
        if (v.IsString()) {
            neighbors.push_back(v.GetString());
        }
    }

    return neighbors;
}

// BFS Traversal
void bfs(const string& start_node, int max_depth) {
    auto start_time = chrono::high_resolution_clock::now();

    queue<Node> q;
    unordered_set<string> visited;

    q.push({start_node, 0});
    visited.insert(start_node);

    cout << "BFS Traversal from \"" << start_node << "\" up to depth " << max_depth << ":\n";

    while (!q.empty()) {
        Node current = q.front();
        q.pop();
        cout << current.name << " (Depth: " << current.depth << ")\n";

        if (current.depth < max_depth) {
            vector<string> neighbors = get_neighbors(current.name);
            cout << "Neighbors of " << current.name << ": ";
            for (const auto& neighbor : neighbors) {
                cout << neighbor << " ";
            }
            cout << "\n";

            for (const auto& neighbor : neighbors) {
                if (visited.find(neighbor) == visited.end()) {
                    q.push({neighbor, current.depth + 1});
                    visited.insert(neighbor);
                }
            }
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    cout << "Execution Time: " << elapsed.count() << " seconds\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <actor/movie_name> <depth>\n";
        return 1;
    }

    string start_node = argv[1];
    int max_depth = stoi(argv[2]);

    bfs(start_node, max_depth);

    return 0;
}

