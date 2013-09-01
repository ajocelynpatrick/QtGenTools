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


class QtTool {
public:

	void init(const std::string& qtBinPath) {
		exePath_ = exePath(qtBinPath);
		if (exePath_.find(' ') != std::string::npos) {
			exePath_ = std::string("\"") + exePath_;
			exePath_.push_back('"');
		}
	}

	virtual std::string exePath(const std::string& qtBinPath) =0;
	virtual bool isFileInput(const std::string& inFile) =0;
	virtual std::string getOutFilename (const std::string& inFilename) =0;

	virtual bool needsToRun(const std::string& inFile, const std::string& outFile);
	
	virtual bool runIfNeeded(const std::string& inFile, const std::string& outFile);

	void setCmdOpts(const std::string& cmdOpts) {
		cmdOpts_ = cmdOpts;
	}


protected:

	std::string exePath_;
	std::string cmdOpts_;
};



class QtMocTool : public QtTool {
public:

	virtual std::string exePath(const std::string& qtBinPath) override;
	virtual bool isFileInput(const std::string& inFile) override;
	virtual std::string getOutFilename (const std::string& inFilename) override;

};



class QtUicTool : public QtTool {
public:

	virtual std::string exePath(const std::string& qtBinPath) override;
	virtual bool isFileInput(const std::string& inFile) override;
	virtual std::string getOutFilename (const std::string& inFilename) override; 

};



class QtRccTool : public QtTool {
public:

	virtual std::string exePath(const std::string& qtBinPath) override;
	virtual bool isFileInput(const std::string& inFile) override;
	virtual std::string getOutFilename (const std::string& inFilename) override;
	virtual bool needsToRun(const std::string& inFile, const std::string& outFile) override;

};
