#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;

size_t WriteCallback(void* content, size_t size, size_t memv, std::string* output) {
    size_t totalSize = size * memv;
    output->append((char*)content, totalSize);
    return totalSize;
}

int main() {

    CURL* curlSend = curl_easy_init();
    CURL* curlFetch = curl_easy_init();

    if (!curlSend || !curlFetch) {
        std::cerr << "curl lib failed." << std::endl;
        return 1;
    }

    // tor proxy settings for message or server request send/fetch
    curl_easy_setopt(curlSend, CURLOPT_PROXY, "socks5h://127.0.0.1:9150");
    curl_easy_setopt(curlFetch, CURLOPT_PROXY, "socks5h://127.0.0.1:9150");


    // tor proxy test
    CURL* onionTest = curl_easy_init();
    if (!onionTest) {
        std::cerr << "curl failed." << std::endl;
        return 1;
    }

    curl_easy_setopt(onionTest, CURLOPT_PROXY, "socks5h://127.0.0.1:9150");
    curl_easy_setopt(onionTest, CURLOPT_URL, "http://onionurl/api.php?action=fetch");
    std::string response;
    curl_easy_setopt(onionTest, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(onionTest, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(onionTest, CURLOPT_TIMEOUT, 5L);
    CURLcode oni = curl_easy_perform(onionTest);
    if (oni != CURLE_OK)
    {
        std::cout << "Open Tor Browser. " << curl_easy_strerror(oni) << std::endl;
        Sleep(2000);
        return 0;
    }
    else {
        std::cout << "Tor proxy connected successfully." << std::endl;
    }




    // register user
    std::string username;
    std::cout << "enter ur nickname : ";
    std::getline(std::cin, username);
    {
        std::string postFields = "name=" + username;
        curl_easy_setopt(curlSend, CURLOPT_URL, "http://onionurl/api.php?action=register");
        curl_easy_setopt(curlSend, CURLOPT_POSTFIELDS, postFields.c_str());

        CURLcode result = curl_easy_perform(curlSend);
        if (result != CURLE_OK) {
            std::cerr << "register request failed: " << curl_easy_strerror(result) << std::endl;
            Sleep(2000);
            return 0;

        }
    }

    std::thread fetchThread([&]() {
        while (true) {
            std::string response;
            response.clear();

            curl_easy_setopt(curlFetch, CURLOPT_URL, "http://onionurl/api.php?action=fetch");
            curl_easy_setopt(curlFetch, CURLOPT_HTTPGET, 1L);
            curl_easy_setopt(curlFetch, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curlFetch, CURLOPT_WRITEDATA, &response);

            CURLcode result = curl_easy_perform(curlFetch);
            if (result != CURLE_OK) {
                std::cerr << "Fetch request failed: " << curl_easy_strerror(result) << std::endl;
            }
            else if (response.empty()) {
                std::cerr << "Fetch response empty!" << std::endl;
            }

#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif



            try {
                auto messages = json::parse(response);

                for (auto& msg : messages) {
                    std::cout << "[" << msg["time"].get<std::string>() << "] ";
                    std::cout << msg["name"].get<std::string>() << ": ";
                    std::cout << msg["message"].get<std::string>() << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "json error: " << e.what() << std::endl;
                std::cout << response << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        });

    while (true) {
        std::string message;
        std::getline(std::cin, message);
        if (message.empty()) continue;

        std::string postFields = "name=" + username + "&message=" + message;
        curl_easy_setopt(curlSend, CURLOPT_URL, "http://onionurl/api.php?action=send");
        curl_easy_setopt(curlSend, CURLOPT_POSTFIELDS, postFields.c_str());

        CURLcode result = curl_easy_perform(curlSend);
        if (result != CURLE_OK) {
            std::cerr << "Send request failed: " << curl_easy_strerror(result) << std::endl;
        }
    }

    fetchThread.join();
    curl_easy_cleanup(curlFetch);
    curl_easy_cleanup(curlSend);

    return 0;
}
