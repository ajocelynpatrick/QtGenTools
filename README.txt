QtGenTools - Qt Code Generation Tools Utility


Author: Remi THEBAULT - remi.thebault@gmail.com

Copyright (C) 2013, Remi THEBAULT


Description:
------------

	Small and simple command line utility that calls moc, uic and rcc.
	It parses recursively an input directory and look for input files.
	Generated code files are written to the output directory.
	For each input file, it compares last modification date and do not
	modify the output file if not necessary. For rcc, if does the same
	comparison with the input files too.
	
	This utility is useful if you want to use Qt without qmake, for example
	with Microsoft Visual C++ Express.


	Moc:
		input files must be header (with extension .h, .hpp, .hh, .hxx) and
		contain the string 'Q_OBJECT'. Output files are C++ source prefixed
		by "mo_" and with extension ".cc"
	
	Uic:
		Input files must have extension .ui. Output files are C++ headers prefixed
		by "ui_" and with extension ".h"
	
	Rcc:
		Input files must have extension .qrc. Output files are C++ source prefixed
		by "rc_" and with extension ".cc"
	
	
	The generated files can be afterwards added in your IDE project or build system.
	
	
	
Synopsis:
---------
	
	Usage: QtGenTools --inD=<IN_DIR> --outD=<OUT_DIR> [Options]

	Options:
	  --inD=<in_dir>    Specify the input directory (mandatory)
	  --outD=<out_dir>  Specify the output directory (mandatory)
	  --mocOpts=<opts>  Command line options given to moc
	  --uicOpts=<opts>  Command line options given to uic
	  --rccOpts=<opts>  Command line options given to rcc



Example of use:
---------------

	QtGenTools --inD=YourProjectDir --outD=YourProjectDir/QtGen
				--mocOpts=-b"stdafx.h"
	
	This command will parse YourProjectDir, find all Qt input files,
	run the appropriate tools and place generated code files in 
	YourProjectDir/QtGen.
	In addition, with this example, meta-object files will have 
	#include "stdafx.h" as first directive, and be compatible with
	precompiled headers.
	
	This command can be added as a pre build step in your favorite IDE or
	build system in order to updated generated files when needed.
	
Enjoy!
