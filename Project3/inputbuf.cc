/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cstdio>

#include "inputbuf.h"

using namespace std;

bool InputBuffer::EndOfInput()
{ 
    if (!input_buffer.empty()) 
	return false;
    else
    {   
        return cin.eof();
    }
}

char InputBuffer::UngetChar(char c)
{
    if (c != EOF)
        input_buffer.push_back(c);;
    return c;
}

bool InputBuffer::GetChar(char& c)
{
    if (!input_buffer.empty()) {
        c = input_buffer.back();
        input_buffer.pop_back();
        return true;
    } else {
        cin.get(c);
	if (!cin.eof())
		return true;
	else
		return false;
    }
}

string InputBuffer::UngetString(string s)
{
    for (int i = 0; i < s.size(); i++)
        input_buffer.push_back(s[s.size()-i-1]);
    return s;
}
