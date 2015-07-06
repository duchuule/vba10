#ifndef STRINGHELPER_H_
#define STRINGHELPER_H_

#include <string>
#include <vector>

using namespace std;

string &strreplace(string &input, char oldChar, char newChar);

vector<string> &strSplitLines(string &input, vector<string> &v);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

void StrToUpper(string &input);

void splitFilePath(wstring &filepath, wstring &folderpath, wstring &filename, wstring &filenamenoext, wstring &ext);

string trim(const std::string& str, const std::string& whitespace = " \t");

string reduce(const std::string& str, const std::string& fill = " ", const std::string& whitespace = " \t");

#endif