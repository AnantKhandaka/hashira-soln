#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

class JSONParser {
private:
    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
    
    std::string extractStringValue(const std::string& line) {
        size_t colonPos = line.find(":");
        if (colonPos == std::string::npos) return "";
        
        size_t startQuote = line.find("\"", colonPos);
        if (startQuote == std::string::npos) return "";
        
        size_t endQuote = line.find("\"", startQuote + 1);
        if (endQuote == std::string::npos) return "";
        
        return line.substr(startQuote + 1, endQuote - startQuote - 1);
    }
    
    std::string extractNumberValue(const std::string& line) {
        size_t colonPos = line.find(":");
        if (colonPos == std::string::npos) return "";
        
        std::string value = line.substr(colonPos + 1);
        value = trim(value);
        
        if (!value.empty() && value.back() == ',') {
            value.pop_back();
        }
        
        return value;
    }

public:
    struct ShareData {
        std::string base;
        std::string value;
    };
    
    struct JSONData {
        int n = 0, k = 0;
        std::map<int, ShareData> shares;
    };
    
    JSONData parseFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return JSONData();
        }
        
        JSONData data;
        std::string line;
        std::string currentShare = "";
        ShareData tempShare;
        bool inShare = false;
        
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line == "{" || line == "}") continue;
            
            if (line.find("\"n\"") != std::string::npos) {
                std::string nStr = extractNumberValue(line);
                data.n = std::stoi(nStr);
            }
            else if (line.find("\"k\"") != std::string::npos) {
                std::string kStr = extractNumberValue(line);
                data.k = std::stoi(kStr);
            }
            else {
                for (int i = 1; i <= 50; i++) {
                    std::string pattern = "\"" + std::to_string(i) + "\":";
                    if (line.find(pattern) != std::string::npos) {
                        if (inShare && !currentShare.empty()) {
                            data.shares[std::stoi(currentShare)] = tempShare;
                        }
                        currentShare = std::to_string(i);
                        tempShare = ShareData();
                        inShare = true;
                        break;
                    }
                }
                
                if (inShare) {
                    if (line.find("\"base\"") != std::string::npos) {
                        tempShare.base = extractStringValue(line);
                    }
                    else if (line.find("\"value\"") != std::string::npos) {
                        tempShare.value = extractStringValue(line);
                    }
                    else if (line.find("}") != std::string::npos && line.find("{") == std::string::npos) {
                        if (!currentShare.empty()) {
                            data.shares[std::stoi(currentShare)] = tempShare;
                        }
                        inShare = false;
                        currentShare = "";
                    }
                }
            }
        }
        
        if (inShare && !currentShare.empty()) {
            data.shares[std::stoi(currentShare)] = tempShare;
        }
        
        file.close();
        return data;
    }
};

class ShamirSecretSharing {
private:
    struct Point {
        int x;
        long long y;
    };
    
    std::vector<Point> points;
    int k;

public:
    ShamirSecretSharing(int min_shares) : k(min_shares) {}
    
    long long baseToDecimal(const std::string& value, int base) {
        long long result = 0;
        long long power = 1;
        
        for (int i = value.length() - 1; i >= 0; i--) {
            char digit = value[i];
            int digit_value;
            
            if (digit >= '0' && digit <= '9') {
                digit_value = digit - '0';
            } else if (digit >= 'A' && digit <= 'Z') {
                digit_value = digit - 'A' + 10;
            } else if (digit >= 'a' && digit <= 'z') {
                digit_value = digit - 'a' + 10;
            } else {
                return 0;
            }
            
            if (digit_value >= base) {
                return 0;
            }
            
            result += digit_value * power;
            power *= base;
        }
        
        return result;
    }
    
    void addPoint(int x, const std::string& baseStr, const std::string& value) {
        int base = std::stoi(baseStr);
        long long y = baseToDecimal(value, base);
        points.push_back({x, y});
    }
    
    long long findSecret() {
        if (points.size() < k) {
            return 0;
        }
        
        std::vector<Point> selected_points(points.begin(), points.begin() + k);
        double result = 0.0;
        
        for (int i = 0; i < k; i++) {
            double term = selected_points[i].y;
            double lagrange_coefficient = 1.0;
            
            for (int j = 0; j < k; j++) {
                if (i != j) {
                    lagrange_coefficient *= (double)(-selected_points[j].x) / 
                                           (selected_points[i].x - selected_points[j].x);
                }
            }
            
            result += term * lagrange_coefficient;
        }
        
        return (long long)round(result);
    }
};

int main() {
    try {
        JSONParser parser;
        auto jsonData = parser.parseFile("test2.json");
        
        ShamirSecretSharing sss(jsonData.k);
        
        for (const auto& share : jsonData.shares) {
            sss.addPoint(share.first, share.second.base, share.second.value);
        }
        
        long long secret = sss.findSecret();
        std::cout << secret << std::endl;
        
    } catch (...) {
        // Silent error handling
    }
    
    return 0;
}