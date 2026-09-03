#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>











class EnvConfig
{
public:
    
    
    
    static void loadDotEnv(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return;
        }

        std::string line;
        while (std::getline(file, line))
        {
            std::string trimmed = trim(line);
            if (trimmed.empty() || trimmed[0] == '#')
            {
                continue;
            }

            auto eqPos = trimmed.find('=');
            if (eqPos == std::string::npos)
            {
                continue;
            }

            std::string key = trim(trimmed.substr(0, eqPos));
            std::string value = trim(trimmed.substr(eqPos + 1));

            
            if (value.size() >= 2 &&
                ((value.front() == '"' && value.back() == '"') ||
                 (value.front() == '\'' && value.back() == '\'')))
            {
                value = value.substr(1, value.size() - 2);
            }

            if (key.empty())
            {
                continue;
            }

            
            
            if (std::getenv(key.c_str()) == nullptr)
            {
#ifdef _WIN32
                _putenv_s(key.c_str(), value.c_str());
#else
                setenv(key.c_str(), value.c_str(), 0);
#endif
            }
        }
    }

    
    static std::string getOrDefault(const std::string& key, const std::string& fallback)
    {
        const char* val = std::getenv(key.c_str());
        if (val == nullptr || std::string(val).empty())
        {
            return fallback;
        }
        return std::string(val);
    }

    
    
    
    static std::string require(const std::string& key)
    {
        const char* val = std::getenv(key.c_str());
        if (val == nullptr || std::string(val).empty())
        {
            throw std::runtime_error("Required environment variable '" + key +
                                      "' is not set. Copy backend/.env.example to "
                                      "backend/.env and fill in real values.");
        }
        return std::string(val);
    }

    
    
    
    
    static std::string expandEnvVars(const std::string& text)
    {
        std::string result;
        result.reserve(text.size());

        size_t i = 0;
        while (i < text.size())
        {
            if (text[i] == '$' && i + 1 < text.size() && text[i + 1] == '{')
            {
                size_t close = text.find('}', i + 2);
                if (close != std::string::npos)
                {
                    std::string varName = text.substr(i + 2, close - (i + 2));
                    const char* val = std::getenv(varName.c_str());
                    result += (val != nullptr) ? val : "";
                    i = close + 1;
                    continue;
                }
            }
            result += text[i];
            ++i;
        }
        return result;
    }

private:
    static std::string trim(const std::string& s)
    {
        size_t start = 0;
        size_t end = s.size();
        while (start < end && std::isspace(static_cast<unsigned char>(s[start])))
        {
            ++start;
        }
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        {
            --end;
        }
        return s.substr(start, end - start);
    }
};
