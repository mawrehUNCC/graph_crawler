#include <iostream>
#include <string>
#include <queue>
#include <unordered_set>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

// Function to handle the curl response and write it to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch the neighbors of a node from the API
bool get_neighbors(const std::string& node, std::unordered_set<std::string>& neighbors) {
    CURL* curl;
    CURLcode res;
    std::string url = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + node;
    std::string response_body;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

        // Perform the request
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        // Print the raw response for debugging
        std::cout << "Raw response: " << response_body << std::endl;

        // Parse the JSON response
        rapidjson::Document doc;
        doc.Parse(response_body.c_str());

        if (doc.HasParseError()) {
            std::cerr << "Error parsing JSON: " << rapidjson::GetParseError_En(doc.GetParseError()) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        // Ensure the response contains a valid JSON object and the 'neighbors' array
        if (doc.IsObject() && doc.HasMember("neighbors") && doc["neighbors"].IsArray()) {
            const rapidjson::Value& neighbors_array = doc["neighbors"];
            for (rapidjson::SizeType i = 0; i < neighbors_array.Size(); i++) {
                neighbors.insert(neighbors_array[i].GetString());
            }
        } else {
            std::cerr << "Invalid JSON structure or missing 'neighbors' array!" << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return true;
}

// Function to perform BFS traversal
void bfs_traversal(const std::string& start_node, int depth) {
    std::queue<std::pair<std::string, int>> q;
    std::unordered_set<std::string> visited;

    // Add the start node to the queue
    q.push({start_node, 0});
    visited.insert(start_node);

    while (!q.empty()) {
        std::string current_node = q.front().first;
        int current_depth = q.front().second;
        q.pop();

        // If we've reached the desired depth, stop processing neighbors
        if (current_depth >= depth) continue;

        std::unordered_set<std::string> neighbors;
        if (get_neighbors(current_node, neighbors)) {
            for (const auto& neighbor : neighbors) {
                if (visited.find(neighbor) == visited.end()) {
                    std::cout << "Visited: " << neighbor << std::endl;
                    visited.insert(neighbor);
                    q.push({neighbor, current_depth + 1});
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./bfs_traversal <start_node> <depth>" << std::endl;
        return 1;
    }

    std::string start_node = argv[1];
    int depth = std::stoi(argv[2]);

    bfs_traversal(start_node, depth);

    return 0;
}
