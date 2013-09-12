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

#ifdef _WIN32
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <string>
#include <tuple>


namespace fu {


#ifdef _WIN32
	const char pathSep = '\\';
	const char pathVarSep = ';';
#else
	const char pathSep = '/';
	const char pathVarSep = ':';
#endif





	template<typename CharT>
	inline std::tuple<std::basic_string<CharT>, std::basic_string<CharT> >
	splitExt(const std::basic_string<CharT>& filename)
	{
		size_t pos = filename.rfind(CharT('.'));
		if (pos == std::basic_string<CharT>::npos) {
			return std::make_tuple(filename, std::basic_string<CharT>());
		}

		return std::make_tuple(filename.substr(0, pos), filename.substr(pos));
	}



	inline bool exists(const std::string& path)
	{
#ifdef _WIN32
		return (INVALID_FILE_ATTRIBUTES != GetFileAttributes(path.c_str()));
#else
		struct stat st;
		int res = stat(path.c_str(), &st);
		return res == 0;
#endif
	}

	inline bool isFile(const std::string& path)
	{
#ifdef _WIN32
		DWORD attr = GetFileAttributes(path.c_str());
		return (attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY);
#else
		struct stat st;
		int res = stat(path.c_str(), &st);
		return (res==0 && S_ISREG(st.st_mode));
#endif
	}

	inline bool isDir(const std::string& path)
	{
#ifdef _WIN32
		DWORD attr = GetFileAttributes(path.c_str());
		return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
		struct stat st;
		int res = stat(path.c_str(), &st);
		return (res==0 && S_ISDIR(st.st_mode));
#endif
	}


	inline bool mkDir(const std::string& path, const unsigned int& perm=0755)
	{
#ifdef _WIN32
		return FALSE != CreateDirectory(path.c_str(), NULL);
#else
		return 0 == mkdir(path.c_str(), perm);
#endif
	}


	inline bool rm(const std::string& path)
	{
#ifdef _WIN32
		return FALSE != DeleteFile(path.c_str());
#else
		return 0 == unlink(path.c_str());
#endif
	}



	template<typename CharT>
	std::basic_string<CharT> parentDir(const std::basic_string<CharT>& path)
	{
		size_t startPos = std::basic_string<CharT>::npos;
		if (path.back() == CharT(pathSep)) {
			startPos = path.size()-2;
		}
		size_t pos = path.rfind(CharT(pathSep), startPos);

		return path.substr(0, pos+1);
	}






	template<class OutputIt>
	void listDir(std::string dir, OutputIt out, bool reportDirs = false
#ifdef _WIN32
		,std::string filter="*"
#endif
		)
	{

		if (dir.back() != pathSep) dir.push_back(pathSep);

#ifdef _WIN32
		WIN32_FIND_DATA ffd;
		HANDLE hFile;

		dir += filter;

		hFile = FindFirstFile(dir.c_str(), &ffd);
		if (INVALID_HANDLE_VALUE == hFile) {
			return;
		}

		do {

			if (!reportDirs && ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
				continue;
			}
			if (ffd.cFileName[0] == '.') continue;

			*out = std::string(ffd.cFileName);
			++out;

		}
		while(FindNextFile(hFile, &ffd) != 0);

		FindClose(hFile);
#else
		DIR *dp;
		dp = opendir(dir.c_str());
		if (dp != NULL) {
			struct dirent *ep;
			while ((ep = readdir(dp))) {
				if (ep->d_name[0] == '.') continue;

				std::string path(ep->d_name);
				std::string completePath = dir + path;

				if (reportDirs) {
					*out = path;
					++out;
				}
				else if (!isDir(completePath)) {
					*out = path;
					++out;
				}
			}
			closedir(dp);
		}
#endif
	}



	template<class ActionT>
	void walk(std::string root, ActionT& action, bool reportDirs=false)
	{

		if (root.back() != pathSep) root.push_back(pathSep);

#ifdef _WIN32
		WIN32_FIND_DATA ffd;
		HANDLE hFile;

		std::string rootPattern = root;
		rootPattern.push_back('*');

		hFile = FindFirstFile(rootPattern.c_str(), &ffd);
		if (INVALID_HANDLE_VALUE == hFile) {
			return;
		}

		do {
			std::string filename = std::string(ffd.cFileName);
			if (filename.front() == '.') continue;

			if (ffd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
				if (reportDirs) {
					action(root, filename, true);
				}
				walk(root + filename, action, reportDirs);
			}
			else {
				action(root, filename, false);
			}
		}
		while(FindNextFile(hFile, &ffd) != 0);

		FindClose(hFile);
#else
		DIR *dp;
		dp = opendir(root.c_str());
		if (dp != NULL) {
			struct dirent *ep;
			while ((ep = readdir(dp))) {
				if (ep->d_name[0] == '.') continue;

				std::string path(ep->d_name);
				std::string completePath = root + path;

				if (isDir(completePath)) {
					if (reportDirs) {
						action(root, path, true);
					}
					walk(root + path, action, reportDirs);
				}
				else {
					action(root, path, false);
				}
			}
			closedir(dp);
		}
#endif
	}

}
