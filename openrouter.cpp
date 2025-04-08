#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append((char *)contents, totalSize);
    return totalSize;
}
class AIchatbot
{
private:
    std::string modelName;
    std::string apiKey;
    CURL *curl;

public:
    AIchatbot(const std::string &apiKey, const std::string &modelName = "deepseek/deepseek-r1:free") : apiKey(apiKey), modelName(modelName)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
    }
    ~AIchatbot()
    {
        curl_global_cleanup();
        curl_easy_cleanup(curl);
    }

    std::string getResponse(std::string &prompt)
    {
        if (!curl)
            return "Error cant init CURL";
        std::string response;
        std::string url = "https://openrouter.ai/api/v1/chat/completions";
        json payload;
        payload["model"] = modelName;
        payload["messages"] = json::array();
        payload["messages"].push_back({{"role", "user"}, {"content", prompt}});
        std::string payloadStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
        headers = curl_slist_append(headers, "Referer: Your-App-URL");
        headers = curl_slist_append(headers, "X-Title: Your-App-Name");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (httpCode != 200)
        {
            return "HTTP Error " + std::to_string(httpCode) + " " + response;
        }

        if (res != CURLE_OK)
        {
            return "Curl Error " + std::string(curl_easy_strerror(res));
        }
        try
        {
            json parse = json::parse(response);
            if (parse.contains("error"))
            {
                return "API Error: " + parse["error"]["message"].get<std::string>();
            }
            return parse["choices"][0]["message"]["content"].get<std::string>();
        }
        catch (const std::exception &e)
        {
            return "JSON Parse Error: " + std::string(e.what());
        }
    }
};

int main()
{

    std::string apiKey = "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";
    AIchatbot aibot(apiKey, "deepseek/deepseek-r1:free");
    std::string userInput;
    while (true)
    {
        std::cout << "Enter the prompt: " ;
        getline(std::cin, userInput);
        if (userInput == "exit" || userInput == "quit")
            break;
        std::string response = aibot.getResponse(userInput);
        std::cout << "AI>> " << response << std::endl;
    }
    return 0;
}