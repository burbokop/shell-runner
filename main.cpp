#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <list>
#include <algorithm>
#include <map>
#include <dirent.h>
#include <string.h>
#include <ustat.h>
#include <iostream>

namespace add {

#ifdef __unix__
const char separator = '/';
const std::string separatorString = "/";
#else
const char separator = '\\';
const std::string separatorString = "\\";
#endif

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


std::string constrainPath(const std::string &path) {
    if(path.size() <= 0)
        return "";

    auto results = split(path, separator);

    std::list<std::string> stack;
    for(auto&& r : results) {
        if(r == "..") {
            stack.pop_back();
        } else if(r == ".") {

        } else if(r == "") {

        } else {
            stack.push_back(r);
        }
    }

    std::string result = (path[0] == separator) ? separatorString : "";
    int i = 0;
    for(auto&& s : stack) {
        result += ((i == 0) ? "" : separatorString) + s;
        i++;
    }
    return result;
}


std::pair<std::string, std::string> splitIntoPair(const std::string &s, char delimiter) {
    const auto index = s.find_first_of(delimiter);
    if(index >= 0 && index < s.size())
        return { s.substr(0, index), s.substr(index + 1, s.size() - index) };

    return {};
}

std::string trim(const std::string &str, char symbol = ' ') {
    size_t first = str.find_first_not_of(symbol);
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(symbol);
    return str.substr(first, (last - first + 1));
}

std::string trim(const std::string &str, const std::vector<char> &symbols) {
    std::string result = str;
    for(auto s : symbols) {
        result = trim(result, s);
    }
    return result;
}

std::string removeSymbols(const std::string &string, const std::vector<char> &symbols) {
    std::string result;
    result.reserve(string.size());
    for(auto c : string) {
        if(std::find(symbols.begin(), symbols.end(), c) == symbols.end()) {
            result.push_back(c);
        }
    }
    return result;
}

std::string absolutePath(const std::string &path, const std::string &exe_path) {
    if(exe_path.size() <= 0) return "";
    if(path.size() <= 0) return "";

#ifdef __WIN32__
    if(path.size() > 2 && path[1] == ':' && path[2] == separator) return path;
#else
    if(path[0] == separator) return path;
#endif

    return constrainPath(exe_path + separatorString + ".." + separatorString + path);
}

std::string readFile(std::string path) {
    std::ifstream ifile(path);
    std::string string((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    ifile.close();
    return string;
}


std::vector<std::string> directoryContent(std::string path) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) != nullptr) {
        std::vector<std::string> result;
        while ((ent = readdir (dir)) != nullptr) {
            if(strcmp(ent->d_name, "..") && strcmp(ent->d_name, ".")) {
                result.push_back(ent->d_name);
            }
        }
        closedir (dir);
        return result;
    } else {
        return std::vector<std::string>();
    }
}

std::string fileSufix(std::string string) {
    return string.substr(string.find_last_of('.'), string.length() - 1);
}

std::string concatenatePaths(std::string prefix, std::string string) {
    if(string[0] == '.') string.erase(0, 1);
    if(string[0] != '/') string = '/' + string;
    if(prefix[prefix.length() - 1] == '/') prefix.pop_back();

    return prefix + string;
}

std::string cutPath(std::string string, unsigned level) {
    size_t pos = string.find_last_of('/');
    std::string result = string.substr(0, pos);
    if(level > 1) {
        return cutPath(result, level - 1);
    }
    return result;
}

std::map<std::string, std::string> readAllVof(const std::string &path, char delimiter) {
    std::map<std::string, std::string> result;
    std::ifstream fin;
    fin.open(path, std::ios::in);
    if (fin.is_open()) {
        while(!fin.eof()) {
            std::string line;
            std::getline(fin, line);
            const auto lv = split(line, delimiter);
            if(lv.size() > 1) {
                result[lv[0]] = lv[1];
            }
        }
        fin.close();
    }
    return result;
}

std::string readVof(const std::string &path, const std::string &id, char delimiter) {
    std::ifstream fin;
    fin.open(path, std::ios::in);
    if (fin.is_open()) {
        while(!fin.eof()) {
            std::string line;
            std::getline(fin, line);
            const auto lv = split(line, delimiter);
            if(lv.size() > 1) {
                if (lv[0] == id) {
                    fin.close();
                    return trim(lv[1]);
                }
            }
        }
        fin.close();
    }
    return std::string();
}

void writeVof(const std::string &path, const std::string &id, const std::string &value, char delimiter) {
    bool found = false;
    bool endsWithEndline = false;
    std::ifstream fin;
    std::ofstream fout;
    std::list<std::string> tmp;
    fin.open(path, std::ios::in);
    if (fin.is_open() == false) {
        fout.open(path, std::ios::out);
        fout << id << delimiter << value;
        fout.close();
    } else {
        while(!fin.eof()) {
            std::string line;
            std::getline(fin, line);
            const auto lv = split(line, delimiter);
            if(lv.size() > 1) {
                if (lv[0] == id) {
                    found = true;
                    tmp.push_back(lv[0] + delimiter + value);
                } else {
                    tmp.push_back(lv[0] + delimiter + lv[1]);
                }
            }
        }
        fin.close();
        if(tmp.size() > 0) {
            const auto back = tmp.back();
            if(back.size() > 0) {
                endsWithEndline = (back.back() == '\n');
            }
        }
        if (!found) {
            fout.open(path, std::ios::app);
            if(!endsWithEndline) {
                fout << '\n';
            }
            fout << id + delimiter + value << '\n';
            fout.close();
        } else {
            fout.open(path, std::ios::out);
            for(const auto& line : tmp) {
                fout << line << '\n';
            }
            fout.close();
        }
    }
}

}



int main(int argc, char** argv) {
    if(argc > 0) {
        const std::string exe_location = argv[0];
        const auto config_path = add::absolutePath("./shellrunner.vof", exe_location);
        const auto script_path = add::readVof(config_path, "script", '=');
        if(script_path.size() == 0 || script_path == "__blank__") {
            std::cerr << "error: script not set ( file shellrunner.vof not found in working directory or field \"script\" not found in file )";
            add::writeVof(config_path, "script", "__blank__", '=');
            return -2;
        }
        return system(add::absolutePath(script_path, exe_location).c_str());
    }
    std::cerr << "error: arguments size == 0";
    return -1;
}
