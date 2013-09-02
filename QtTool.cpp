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
#include "QtTool.h"
#include "StringUtils.h"
#include "FileUtils.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>


using namespace std;



bool QtTool::needsToRun(const std::string& inFile, const std::string& outFile)
{
#ifdef _WIN32
	HANDLE hInFile = CreateFile(inFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
	                            OPEN_EXISTING, 0, NULL);
	HANDLE hOutFile = CreateFile(outFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
	                             OPEN_EXISTING, 0, NULL);

	if(hInFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	if(hOutFile == INVALID_HANDLE_VALUE) {
		return true;
	}

	FILETIME lstInMod, lstOutMod;
	GetFileTime(hInFile, NULL, NULL, &lstInMod);
	GetFileTime(hOutFile, NULL, NULL, &lstOutMod);

	CloseHandle(hInFile);
	CloseHandle(hOutFile);

	return CompareFileTime(&lstInMod, &lstOutMod) == 1;
#else
	// TODO implement modification date comparison linux and mac
	return true;
#endif
}





bool QtTool::runIfNeeded(const std::string& inFile, const std::string& outFile)
{
	if (needsToRun(inFile, outFile)) {

		const size_t bufSize = 4096;
		char buf [bufSize];

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		HANDLE outFileH;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		outFileH = CreateFile(outFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
		                      &saAttr, CREATE_ALWAYS, 0, NULL);
		if (outFileH == INVALID_HANDLE_VALUE) {
			throw runtime_error("could not create output file");
		}

		ZeroMemory(&pi, sizeof(pi));
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdOutput = outFileH;
		si.hStdInput = NULL;
		si.dwFlags |= STARTF_USESTDHANDLES;

		ostringstream cmd;
		cmd << exePath_;
		if (cmdOpts_.size() > 0) {
			cmd << " " << cmdOpts_;
		}
		cmd << " " << inFile;
		string cmdOpts = cmd.str();
		for(size_t i=0; i<cmdOpts.size(); ++i) {
			buf[i] = cmdOpts[i];
			if (i == bufSize-1) break;
		}
		size_t i = min(size_t(bufSize-1), cmdOpts.size());
		buf[i] = '\0';

		// Start the child process.
		if( !CreateProcess(
		            NULL,			// module name
		            buf,	        // Command line
		            NULL,           // Process handle not inheritable
		            NULL,           // Thread handle not inheritable
		            TRUE,           // Set handle inheritance to TRUE
		            0,              // No creation flags
		            NULL,           // Use parent's environment block
		            NULL,           // Use parent's starting directory
		            &si,            // Pointer to STARTUPINFO structure
		            &pi )           // Pointer to PROCESS_INFORMATION structure
		  ) {
			cerr << "CreateProcess err " << GetLastError() << "\n";
			throw runtime_error("cannot start process");
			return false;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);

		if (!CloseHandle(pi.hProcess)) {
			cerr << "close handle process\n";
		}
		if (!CloseHandle(pi.hThread)) {
			cerr << "close handle thread\n";
		}
		if (!CloseHandle(outFileH)) {
			cerr << "close handle out file\n";
		}

		return true;
	}
	return false;
}




string QtMocTool::exePath(const string& qtBinPath)
{
	return qtBinPath + "moc.exe";
}



bool QtMocTool::isFileInput(const string& inFile)
{
	if (!su::endsWith(inFile, string(".h")) &&
	        !su::endsWith(inFile, string(".hpp")) &&
	        !su::endsWith(inFile, string(".hh")) &&
	        !su::endsWith(inFile, string(".hxx"))) {
		return false;
	}

	ifstream in (inFile);
	if (!in) return false;

	string line;
	bool found = false;
	const string q_object = "Q_OBJECT";
	while(getline(in, line)) {
		if(line.find(q_object) != string::npos) {
			found = true;
			break;
		}
	}
	in.close();
	return found;
}



std::string QtMocTool::getOutFilename (const string& inFileName)
{
	string base;
	string ext;
	tie(base, ext) = fu::splitExt(inFileName);

	ostringstream oss;
	oss << "mo_" << base << ".cc";
	return oss.str();
}





string QtUicTool::exePath(const string& qtBinPath)
{
	return qtBinPath + "uic.exe";
}



bool QtUicTool::isFileInput(const string& inFile)
{
	if (!su::endsWith(inFile, string(".ui"))) {
		return false;
	}
	return true;
}



string QtUicTool::getOutFilename (const string& inFileName)
{
	string base;
	string ext;
	tie(base, ext) = fu::splitExt(inFileName);

	ostringstream oss;
	oss << "ui_" << base << ".h";
	return oss.str();
}








string QtRccTool::exePath(const string& qtBinPath)
{
	return qtBinPath + "rcc.exe";
}



bool QtRccTool::isFileInput(const string& inFile)
{
	if (!su::endsWith(inFile, string(".qrc"))) {
		return false;
	}
	return true;
}



string QtRccTool::getOutFilename (const string& inFileName)
{
	string base;
	string ext;
	tie(base, ext) = fu::splitExt(inFileName);

	ostringstream oss;
	oss << "rc_" << base << ".cc";
	return oss.str();
}




bool QtRccTool::needsToRun(const std::string& inFile, const std::string& outFile)
{
	if (QtTool::needsToRun(inFile, outFile)) {
		return true;
	}
	bool run = false;

	string baseDir = fu::parentDir(inFile);

	ifstream in (inFile);
	string line;
	while(getline(in, line)) {
		size_t pos = line.find("<file>");
		if (pos != string::npos) {
			line = line.substr(pos + 6);
			pos = line.find("</file>");
			if (pos != string::npos) {
				string fn = line.substr(0, pos);
				if (fn.size() > 0) {
					string resFile = baseDir + fn;
					run = QtTool::needsToRun(resFile, outFile);
					if (run) break;
				}
			}
		}
	}
	in.close();

	return run;
}

