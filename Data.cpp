#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <thread>
#include <mutex>
#include <future>

using namespace std;
class Utils {
public:
    Utils() = default;

    string get_data_type(const string& data_str) {
        if (data_str.front() == '"' && data_str.back() == '"') {
            return "str";
        } else if (data_str.find('.') != string::npos) {
            try {
                stof(data_str);
                return "float";
            } catch (const invalid_argument& e) {
                // Do nothing
            }
        }
        try {
            stoi(data_str);
            return "int";
        } catch (const invalid_argument& e) {
            // Do nothing
        }
        string lowercase = data_str;
        transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
        if (lowercase == "true" || lowercase == "false") {
            return "bool";
        }
        return "str";
    }
};

class Analytics : public Utils {
public:
    explicit Analytics(const map<string, vector<string>>& data)
        : data(data) {}

    map<string, map<string, map<string, vector<int>>>> get_datatypes() {
        vector<string> columns = data.at("columns");
        vector<vector<string>> rows = data.at("rows");
        map<string, map<string, map<string, vector<int>>>> result;

        for (const auto& column : columns) {
            result[column]["types"] = {};
            result[column]["detail"] = {};
            result[column]["rows_types"] = {};
        }

        auto process_row = [&](int row_idx) {
            vector<string> types;
            for (size_t j = 0; j < rows[row_idx].size(); ++j) {
                string dd = get_data_type(rows[row_idx][j]);
                result[columns[j]]["types"][dd] = {};
                if (result[columns[j]]["rows_types"].find(dd) == result[columns[j]]["rows_types"].end()) {
                    result[columns[j]]["rows_types"][dd] = {{"count", {}}, {"rows", {}}};
                }
                result[columns[j]]["rows_types"][dd]["count"].push_back(1);
                result[columns[j]]["rows_types"][dd]["rows"].push_back(row_idx);
                types.push_back(dd);
            }
            for (size_t k = 0; k < rows[row_idx].size(); ++k) {
                for (const auto& _ : result[columns[k]]["types"]) {
                    string dd = get_data_type(rows[row_idx][k]);
                    result[columns[k]]["rows_types"][_]["percent"].push_back(static_cast<float>(result[columns[k]]["rows_types"][_]["count"].size()) / rows.size());
                }
            }
        };

        vector<future<void>> futures;
        for (size_t i = 0; i < rows.size(); ++i) {
            futures.emplace_back(async(launch::async, process_row, i));
        }

        for (auto& future : futures) {
            future.wait();
        }

        return result;
    }

private:
    map<string, vector<std
::string>> data;
};

class Data : public Analytics {
public:
    explicit Data(const string& path)
        : filepath(path) {
        ifstream file(filepath);
        string line;

        while (getline(file, line)) {
            filedata.push_back(line);
        }
        file.close();

        Analytics::Analytics(get_table());
    }

    vector<string> get_columns() {
        return split_and_clear(filedata[0], ',');
    }

    vector<vector<string>> get_rows() {
        vector<vector<string>> rows;
        for (size_t i = 1; i < filedata.size(); ++i) {
            rows.push_back(split_and_clear(filedata[i], ','));
        }
        return rows;
    }

    map<string, vector<string>> get_table() {
        return {{"columns", get_columns()}, {"rows", get_rows()}};
    }

private:
    string filepath;
    vector<string> filedata;

    string clear_arr(const string& x) {
        string result = x;
        result.erase(remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(remove(result.begin(), result.end(), '\''), result.end());
        result.erase(remove(result.begin(), result.end(), '"'), result.end());
        return result;
    }

    vector<string> split_and_clear(const string& s, char delimiter) {
        vector<string> result;
        istringstream iss(s);
        string token;

        while (getline(iss, token, delimiter)) {
            result.push_back(clear_arr(token));
        }
        return result;
    }
};
