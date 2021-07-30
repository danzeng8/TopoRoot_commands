// TopoRoot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <direct.h>
// MSDN recommends against using getcwd & chdir names
#define cwd _getcwd
#define cd _chdir
#else
#include "unistd.h"
#define cwd getcwd
#define cd chdir
#endif

using namespace std;

bool endsWith(std::string const& fileName, std::string const& suffix) {
	if (suffix.length() <= fileName.length()) {
		return (0 == fileName.compare(fileName.length() - suffix.length(), suffix.length(), suffix));
	}
	else {
		return false;
	}
}

// for string delimiter
vector<string> split(string s, string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

bool iequals(const string& a, const string& b)
{
	unsigned int sz = a.size();
	if (b.size() != sz)
		return false;
	for (unsigned int i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}

int main(int argc, char** argv)
{
	string inFile = "";
	string outFile = "";
	string datFile = "";
	float shape = -1;
	float neighborhood = -1;
	float kernel = -1;
	float downsample = 1.0;
	float scaling = 1.0;
	bool raw = false;
	bool multi = false;
	bool plane = false;

	for (int i = 0; i < argc; i++) {
		if (((string)argv[i]).substr(0, 2) == "--") {
			string arg = (string)argv[i];
			arg.erase(0, 2);

			if (iequals(arg, "in")) {
				inFile = argv[i + 1]; //Input skeleton file
			}

			if (iequals(arg, "dat")) {
				datFile = argv[i + 1];
			}

			if (iequals(arg, "out")) {
				outFile = argv[i + 1]; //Output name
			}

			if (iequals(arg, "K")) {
				kernel = stof(argv[i + 1]);
			}

			if (iequals(arg, "S")) {
				shape = stof(argv[i + 1]);
			}

			if (iequals(arg, "N")) {
				neighborhood = stof(argv[i + 1]);
			}

			if (iequals(arg, "D")) {
				downsample = stof(argv[i + 1]);
			}

			if (iequals(arg, "scaling")) {
				scaling = stof(argv[i + 1]);
			}

			if (iequals(arg, "multi")) {
				multi = true;
			}

			if (iequals(arg, "plane")) {
				plane = true;
			}

		}
	}
	
	if (outFile == "") {

		if (endsWith(inFile, ".raw")) {
			outFile = inFile.substr(0, inFile.length() - 4) + "_out";
		}
		else {
			outFile = outFile + "out";
		}

	}
	system("del \"tmp\\downsampled\\*.png\"");
	system("del \"tmp\\preprocess\\*.png\"");
	system("del \"tmp\\holefill\\*.png\"");
	system("del \"tmp\\topoOut\\*.png\"");
	system("del \"tmp\\*.ply\"");
	system("del \"tmp\\*.mrc\"");
	system("del \"tmp\\*.r\"");
	system("del \"tmp\\*.csv\"");


	int width = -1;
	int height = -1;
	int slices = -1;
	if (endsWith(inFile, ".raw")) {
		raw = true;
		std::ifstream inputDat(datFile);
		int lineCt = 0;
		for (std::string line; getline(inputDat, line); )
		{
			lineCt++;
			if (lineCt == 2) {
				std::vector<string> v = split(line, " ");
				width = std::stoi(v[5]);
				height = std::stoi(v[6]);
				slices = std::stoi(v[7]);
			}
		}
	}

	if (raw) {
		if (width == -1 || height == -1 || slices == -1) {
			return 1;
		}
		//char buf[4096];
		//std::cout << "CWD: " << cwd(buf, sizeof buf) << endl;
		cd("fiji-win64/Fiji.app");
		//std::cout << "CWD changed to: " << cwd(buf, sizeof buf) << std::endl;
		string downsampleCommand = "ImageJ-win64 --ij2 --headless --console --run \"macros/downsampleRaw.ijm\" \"input='" + inFile + "', output='../../tmp/downsampled/0000.png',widthIn=" + std::to_string(width) + " , heightIn=" + std::to_string(height) + " , slicesIn=" + std::to_string(slices) + " ,step=" + std::to_string(downsample) + "\"";
		//std::cout << downsampleCommand << endl;
		system(downsampleCommand.c_str());
		cd("../../TopoSimplifier");

	}
	else {
		if (downsample > 1) {
			cd("fiji-win64/Fiji.app");
			string downsampleCommand = "ImageJ-win64 --ij2 --headless --console --run \"macros/downsampled.ijm\" \"input='" + inFile + "', output='../../tmp/downsampled/0000.png' ,step=" + std::to_string(downsample) + "\"";
			system(downsampleCommand.c_str());
			cd("../../TopoSimplifier");
		}
		else {
			cd("TopoSimplifier");
		}
	}

	string preprocess = "";
	if (raw || downsample > 1) {
		preprocess = "TopoSimplifier.exe --in ../tmp/downsampled/ --out ../tmp/preprocess/ --rootMorpho1 --finalTopo 0 --rootShape ../tmp/rootShape.csv --S " + std::to_string(shape) + " --N " + std::to_string(neighborhood) + " --K " + std::to_string(kernel);
	}
	else {
		preprocess = "TopoSimplifier.exe --in " + inFile + " --out ../tmp/preprocess/ --rootMorpho1 --finalTopo 0 --rootShape ../tmp/rootShape.csv --S " + std::to_string(shape) + " --N " + std::to_string(neighborhood) + " --K " + std::to_string(kernel);
	}
	system(preprocess.c_str());
	std::ifstream rootShape("../tmp/rootShape.csv");

	int lineCt = 0;
	for (std::string line; getline(rootShape, line); )
	{
		lineCt++;

		if (lineCt == 1) {
			std::vector<string> v = split(line, ",");
			shape = std::stoi(v[0]);
			if (neighborhood == -1) {
				neighborhood = std::stoi(v[1]);
			}
			if (kernel == -1) {
				kernel = std::stoi(v[2]);
			}
		}
	}

	string topoCommand = "TopoSimplifier.exe --in ../tmp/preprocess/ --out ../tmp/topoOut/ --finalTopo 0 --S " + std::to_string(shape) + " --N " + std::to_string(neighborhood) + " --K " + std::to_string(kernel);
	system(topoCommand.c_str());

	cd("../fiji-win64/Fiji.app");
	string holeFillCommand = "ImageJ-win64 --ij2 --headless --console --run \"macros/holeFill.ijm\" \"input='../../tmp/topoOut/', suffix=\'.png\', threshold=" + std::to_string(shape) + "\"";
	system(holeFillCommand.c_str());
	cd("../../TopoSimplifier");
	string pngToMrcCommand = "TopoSimplifier.exe --in ../tmp/holefill/ --out ../tmp/topoOut.mrc --finalTopo 0 --K " + std::to_string(shape) + " --S " + std::to_string(shape) + " --N " + std::to_string(shape);
	system(pngToMrcCommand.c_str());
	cd("../voxelcore");
	string voxelcore = "voxelcore -md=vol2ma ../tmp/topoOut.mrc ../tmp/topoOut.ply -fullOrPruned=1";
	system(voxelcore.c_str());
	string voxelcoreMesh = "voxelcore -md=vol2mesh ../tmp/topoOut.mrc " + outFile + " -onlyBndryVts=False";
	system(voxelcoreMesh.c_str());
	cd("../et");
	string et = "EtDev --nogui -ma_file=../tmp/topoOut.ply -r_file=../tmp/topoOut.r -smooth_i=3 -theta_1=0.01 --export_skel";
	system(et.c_str());

	cd("../");
	if (multi) {
		string stemFix = "root_traits_auto.exe --in tmp/topoOut_skel.ply --out " + outFile + "_skel_fixed.ply --fixStem --upperRadius 0.45 --lowerRadius 0.3 --multi";
		system(stemFix.c_str());
	}
	else {
		string stemFix = "root_traits_auto.exe --in tmp/topoOut_skel.ply --out " + outFile + "_skel_fixed.ply --fixStem --upperRadius 0.9 --lowerRadius 0.2 --radiusTolerance 1.6";
		system(stemFix.c_str());
	}
	string skelMST = "skeletonMST.exe --in "+outFile+"_skel_fixed.ply --out tmp/acyclic.ply --gaussian 2.5";
	system(skelMST.c_str());

	if (multi) {
		string hierarchy = "root_traits_auto.exe --in tmp/acyclic.ply --out " + outFile + ".ply --traits " + outFile + "_traits.csv --stemExists " + std::to_string(scaling) + " --annotation " + outFile + "_annotations.txt";
		if (plane) {
			hierarchy += " --plane";
		}
		system(hierarchy.c_str());
	}
	else {
		string hierarchy = "root_traits_auto.exe --in tmp/acyclic.ply --out " + outFile + ".ply --traits " + outFile + "_traits.csv --stemExists --scaling " + std::to_string(scaling) + " --radiusTolerance 1.6 --annotation " + outFile + "_annotations.txt";
		if (plane) {
			hierarchy += " --plane";
		}

		system(hierarchy.c_str());
	}
	
}


