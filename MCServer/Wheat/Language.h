#pragma once
#include <fstream>
#include <map>
#include <string>

class Language
{
public:
	string operator[](const string key) { return this->languageMap[key]; };

	std::map<std::string, std::string> languageMap; // key - value

	Language() {};
	Language(std::ifstream* file) {
		load(file);
	}
	bool load(std::ifstream* file) {
		if (!file->is_open()) return false;

		while (!file->eof()) {
			char buffer[256];
			file->getline(buffer, 255, '\n');

			int i = 0;
			char c = buffer[i];
			string key = "";
			string value = "";
			bool key_set = false;
			while (c != '\0') {
				if (c == '#') break;
				if (c == '=') {
					key_set = true;
				}
				else {
					if (key_set) value += c;
					else key += c;
				}
				i++;
				c = buffer[i];
			}
			if (key != "" && value != "") {
				languageMap[key] = value;
			}
		}
	}
	bool reload(std::ifstream* file) {
		if (!file->is_open()) return false;
		languageMap.clear();
		load(file);
	}
	string valueOf(string key) {
		return languageMap[key];
	}
	string transform(string key, string t1 = "", string t2 = "", string t3 = "", string t4 = "") {
		string str = languageMap[key];
		string result = "";
		string::iterator it = str.begin();

		while (it != str.end()){
			if (*it == '%') {
				it++;
				if (*it == '1') {
					result += t1;
				}
				else if (*it == '2') {
					result += t2;
				}
				else if (*it == '3') {
					result += t3;
				}
				else if (*it == '4') {
					result += t4;
				}
				else if (it == str.end()) {
					break;
				}
			}
			else {
				result += *it;
			}
			it++;
		}
		return result;
	}
} lang;