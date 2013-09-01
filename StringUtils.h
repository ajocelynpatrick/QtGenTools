/*
	Copyright (c) 2013, Remi Thebault
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of the <organization> nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include <string>
#include <functional>
#include <cctype>


// trim from start
template<typename CharT>
inline std::basic_string<CharT>& ltrim(std::basic_string<CharT>& s)
{
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
template<typename CharT>
inline std::basic_string<CharT>& rtrim(std::basic_string<CharT>& s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

// trim from both ends
template<typename CharT>
inline std::basic_string<CharT>& trim(std::basic_string<CharT>& s)
{
	return ltrim(rtrim(s));
}




template<typename CharT>
inline std::basic_string<CharT>& replace(
    std::basic_string<CharT>& s,
    const std::basic_string<CharT>& pattern,
    const std::basic_string<CharT>& repl)
{
	size_t pos = s.find(pattern);
	size_t len = pattern.size();
	while (pos != string::npos) {
		s.replace(pos, len, repl);
		pos = s.find(pattern);
	}
	return s;
}


template<typename CharT, class OutputIt>
inline void split(const std::basic_string<CharT>& s, CharT delim, OutputIt out)
{
	std::basic_istringstream<CharT> iss (s);
	std::basic_string<CharT> elem;
	while (getline(iss, elem, delim)) {
		*out = elem;
		++out;
	}
}



template<typename CharT>
inline bool beginsWith(const std::basic_string<CharT>& str, const std::basic_string<CharT>& pattern)
{
	size_t pos = str.find(pattern);
	return pos == 0;
}



template<typename CharT>
inline bool endsWith(const std::basic_string<CharT>& str, const std::basic_string<CharT>& pattern)
{
	size_t pos = str.find(pattern);
	if (pos == std::basic_string<CharT>::npos) return false;
	return pos == (str.size() - pattern.size());
}