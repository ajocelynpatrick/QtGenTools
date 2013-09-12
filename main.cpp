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
#include "StringUtils.h"
#include "FileUtils.h"
#include "QtTool.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>


using namespace std;



string guessQtBinPath()
{
	string qtBinPath;
	char *qt = getenv("QT5");
	if (qt) {
		qtBinPath = string(qt);
		if (qtBinPath.back() != fu::pathSep) qtBinPath.push_back(fu::pathSep);
		qtBinPath.append("bin");
		qtBinPath.push_back(fu::pathSep);
	}
	else {
		string path = string(getenv("PATH"));
		vector<string> pathComps;
		su::split(path, fu::pathVarSep, back_inserter(pathComps));
		for (size_t i=0; i<pathComps.size(); ++i) {
			string p = pathComps[i];
			if (p.back() != fu::pathSep) p.push_back(fu::pathSep);
#ifdef _WIN32
			string qmake = p + string("qmake.exe");
#else
			string qmake = p + string("qmake");
#endif
			ifstream test (qmake, ios::binary);
			if (test) {
				test.close();
				qtBinPath = p;
				break;
			}
		}
	}
	return qtBinPath;
}



string qtBinPath;
string inD;
string outD;

QtMocTool moc;
QtUicTool uic;
QtRccTool rcc;



class Driver {
public:

	void run() {

		tools_.clear();
		oldFiles_.clear();
		newFiles_.clear();
		genFiles_.clear();
		updatedFiles_.clear();
		untouchedFiles_.clear();
		deletedFiles_.clear();
		errors_.clear();

		tools_.push_back(&moc);
		tools_.push_back(&uic);
		tools_.push_back(&rcc);

		for (int i=0; i<3; ++i) {
			tools_[i]->init(qtBinPath);
		}

		if (inD.back() != fu::pathSep) inD.push_back(fu::pathSep);
		if (outD.back() != fu::pathSep) outD.push_back(fu::pathSep);

		if (!fu::isDir(outD)) {
			if (!fu::mkDir(outD)) {
				throw runtime_error("could not create the output directory");
			}
		}

		fu::listDir(outD, back_inserter(oldFiles_));
		for (size_t i=0; i<oldFiles_.size(); ++i) {
			oldFiles_[i] = outD + oldFiles_[i];
		}

		fu::walk(inD, *this);


		for (size_t i=0; i<oldFiles_.size(); ++i) {
			auto found = find(newFiles_.begin(), newFiles_.end(), oldFiles_[i]);
			if (found == newFiles_.end()) {
				if (fu::rm(oldFiles_[i])) {
					deletedFiles_.push_back(oldFiles_[i]);
				}
				else {
					cerr << "could not delete " << oldFiles_[i] << "\n";
				}
			}
		}


		// printing report
		string sep (79, '-');
		cout << sep << '\n';
		cout << ' ' << inD << '\n';
		cout << sep << '\n';

		if (genFiles_.size() > 0) {
			for (size_t i=0; i<genFiles_.size(); ++i) {
				cout << "generated: " << genFiles_[i] << '\n';
			}
			cout << sep << '\n';
		}

		if (updatedFiles_.size() > 0) {
			for (size_t i=0; i<updatedFiles_.size(); ++i) {
				cout << "updated: " << updatedFiles_[i] << '\n';
			}
			cout << sep << '\n';
		}

		if (deletedFiles_.size() > 0) {
			for (size_t i=0; i<deletedFiles_.size(); ++i) {
				cout << "deleted: " << deletedFiles_[i] << '\n';
			}
			cout << sep << '\n';
		}

		cout << untouchedFiles_.size() << " file(s) were already up-to-date\n";
		cout << genFiles_.size() << " file(s) have been generated\n";
		cout << updatedFiles_.size() << " file(s) have been updated\n";
		cout << deletedFiles_.size() << " file(s) have been deleted\n";

		if (errors_.size() > 0) {
			cout << sep << '\n';
			cout << "error occured when processing the following file(s):\n";
			for (size_t i=0; i<errors_.size(); ++i) {
				cout << errors_[i] << '\n';
			}
		}
	}


	void operator()(const string& root, const string& filename, bool isdir) {

		string inFile = root + filename;

		for (int i=0; i<3; ++i) {
			QtTool *tool = tools_[i];
			if(tool->isFileInput(inFile)) {

				string outFilename = tool->getOutFilename(filename);
				string outFile = outD + outFilename;

				try {
					bool existed = fu::isFile(outFile);

					if (tool->runIfNeeded(inFile, outFile)) {
						if (existed) {
							updatedFiles_.push_back(outFile);
						}
						else {
							genFiles_.push_back(outFile);
						}
					}
					else {
						untouchedFiles_.push_back(outFile);
					}
					newFiles_.push_back(outFile);
				}
				catch (const runtime_error& err) {
					ostringstream out;
					out << filename << ": " << err.what();
					errors_.push_back(out.str());
				}
				break;
			}
		}
	}

private:

	vector<QtTool *> tools_;
	vector<string> oldFiles_;
	vector<string> newFiles_;
	vector<string> genFiles_;
	vector<string> updatedFiles_;
	vector<string> untouchedFiles_;
	vector<string> deletedFiles_;
	vector<string> errors_;
};




void usage(const string& err)
{
	cout << "Usage: QtGenTools --inD=<IN_DIR> --outD=<OUT_DIR> [Options]\n";
	if (err.size() > 0) {
		cout << "Error: " << err << "\n";
	}
	cout << "Options:\n"
	     << "  --inD=<in_dir>    Specify the input directory (mandatory)\n"
	     << "  --outD=<out_dir>  Specify the output directory (mandatory)\n"
	     << "  --mocOpts=<opts>  Command line options given to moc\n"
	     << "  --uicOpts=<opts>  Command line options given to uic\n"
	     << "  --rccOpts=<opts>  Command line options given to rcc\n";
}



int main (int argc, char *argv[])
{

	for (int i=1; i<argc; ++i) {
		string arg = string(argv[i]);
		if (su::beginsWith(arg, string("--qt="))) {
			qtBinPath = arg.substr(5);
			if (qtBinPath.back() == '\\') qtBinPath.push_back('\\');
			qtBinPath += "bin\\";
		}
		else if (su::beginsWith(arg, string("--inD="))) {
			inD = arg.substr(6);
		}
		else if (su::beginsWith(arg, string("--outD="))) {
			outD = arg.substr(7);
		}
		else if (su::beginsWith(arg, string("--mocOpts="))) {
			moc.setCmdOpts(arg.substr(10));
		}
		else if (su::beginsWith(arg, string("--uicOpts="))) {
			uic.setCmdOpts(arg.substr(10));
		}
		else if (su::beginsWith(arg, string("--rccOpts="))) {
			rcc.setCmdOpts(arg.substr(10));
		}
	}

	if (qtBinPath.size() == 0) {
		qtBinPath = guessQtBinPath();
	}

	if (qtBinPath.size() == 0) {
		usage("qt bin path was not found");
		return 1;
	}

	if (!fu::isDir(qtBinPath)) {
		usage("qt bin directory is not valid");
		return 1;
	}

	if (inD.size() == 0) {
		usage("input directory was not specified");
		return 1;
	}

	if (!fu::isDir(inD)) {
		usage("input directory is not valid");
		return 1;
	}

	if (outD.size() == 0) {
		usage("output directory was not specified");
		return 1;
	}

	Driver d;
	d.run();

	return 0;
}
