#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "other.h"

using namespace std;

// function to split a string by a delimiter
vector<string> split(string &str, char delimiter) {
    vector<string> tokens;
    const char* start = str.c_str();  // pointer to the beginning of the string
    const char* end = start + str.length();

    while (start < end) {
        const char* next = find(start, end, delimiter);
        tokens.push_back(string(start, next));
        if (next != end) {
            start = next + 1;
        } else {
            break;
        }
    }

    // if the string ends with a delimiter, add an empty string to the tokens
    if (str.back() == delimiter) {
        tokens.push_back("");
    }

    return tokens;
}

bool match(const vector<string> &sub_levels, size_t sub_index, const vector<string> &msg_levels, size_t msg_index) {
    if (sub_index == sub_levels.size()) {
        return msg_index == msg_levels.size();
    }

    if (msg_index == msg_levels.size()) {
        for (size_t i = sub_index; i < sub_levels.size(); i++) {
            if (sub_levels[i] != "*") return false;
        }
        return true;
    }

    if (sub_levels[sub_index] == "*") {
        return match(sub_levels, sub_index + 1, msg_levels, msg_index) || match(sub_levels, sub_index, msg_levels, msg_index + 1);
    }

    if (sub_levels[sub_index] == "+") {
        if (msg_index < msg_levels.size()) {
            return match(sub_levels, sub_index + 1, msg_levels, msg_index + 1);
        }
        return false; // fail if there's no message level to match the '+'
    }

    if (sub_levels[sub_index] == msg_levels[msg_index]) {
        return match(sub_levels, sub_index + 1, msg_levels, msg_index + 1);
    }

    return false;
}


// function to check if two topics correspond
bool topics_correspond(string msg_topic, string sub_topic) {
    vector<string> sub_levels = split(sub_topic, '/');
    vector<string> msg_levels = split(msg_topic, '/');
    return match(sub_levels, 0, msg_levels, 0);
}
