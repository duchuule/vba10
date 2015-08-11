#include "stringhelper.h"
#include <sstream>

using namespace std;

string &strreplace(string &input, char oldChar, char newChar)
{
	for (int i = 0; i < input.length(); i++)
	{
		if(input.at(i) == oldChar)
		{
			input.replace(i, 1, 1, newChar);
		}
	}
	return input;
}

wstring &strreplace(wstring &input, char oldChar, char newChar)
{
	for (int i = 0; i < input.length(); i++)
	{
		if (input.at(i) == oldChar)
		{
			input.replace(i, 1, 1, newChar);
		}
	}
	return input;
}

vector<string> &strSplitLines(string &input, vector<string> &v)
{
	string line;
	stringstream wss(input);
	while(getline(wss, line, '\r'))
	{
		// Ignore empty lines
		if(line.length() > 0)
		{
			v.push_back(line);
		}
	}

	return v;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

void replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

void StrToUpper(string &input)
{
	for (int i = 0; i < input.length(); i++)
	{
		input.replace(i, 1, 1, toupper(input.at(i)));
	}
}

void splitFilePath(wstring &filepath, wstring &folderpath, wstring &filename, wstring &filenamenoext, wstring &ext)
{
	size_t index = filepath.find_last_of(L"/\\");
	folderpath = filepath.substr(0, index);
	filename = filepath.substr(index + 1);

	index = filename.find_last_of('.');
	filenamenoext = filename.substr(0, index);
	ext = filename.substr(index + 1);

}

std::string trim(const std::string& str,
	const std::string& whitespace)
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
	const std::string& fill,
	const std::string& whitespace)
{
	// trim first
	auto result = trim(str, whitespace);

	// replace sub ranges
	auto beginSpace = result.find_first_of(whitespace);
	while (beginSpace != std::string::npos)
	{
		const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
		const auto range = endSpace - beginSpace;

		result.replace(beginSpace, range, fill);

		const auto newStart = beginSpace + fill.length();
		beginSpace = result.find_first_of(whitespace, newStart);
	}

	return result;
}
