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

void StrToUpper(string &input)
{
	for (int i = 0; i < input.length(); i++)
	{
		input.replace(i, 1, 1, toupper(input.at(i)));
	}
}