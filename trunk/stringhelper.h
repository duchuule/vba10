#ifndef STRINGHELPER_H_
#define STRINGHELPER_H_

#include <string>
#include <vector>



std::string &strreplace(std::string &input, char oldChar, char newChar);
std::wstring &strreplace(std::wstring &input, char oldChar, char newChar);

std::vector<std::string> &strSplitLines(std::string &input, std::vector<std::string> &v);

void replaceAll(std::string& str, const std::string& from, const std::string& to);

void StrToUpper(std::string &input);

void splitFilePath(std::wstring &filepath, std::wstring &folderpath, std::wstring &filename, std::wstring &filenamenoext, std::wstring &ext);

std::string trim(const std::string& str, const std::string& whitespace = " \t");

std::string reduce(const std::string& str, const std::string& fill = " ", const std::string& whitespace = " \t");

#endif