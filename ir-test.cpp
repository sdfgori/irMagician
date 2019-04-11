#include "ir.hpp"

static const char *usage =
"irMagician CLI utility.\n"
"\n"
"Usage: %s [-hcspv] [-f <file>] [-n <file prefixes>] [-d]\n"
"\n"
" Arguments:\n"
"   -h - this help\n"
"   -c - capture IR data\n"
"   -s -f <output_file> - save IR data(json)\n"
"   -p [-f <input_file>] - play IR data(json)\n"
"   -n - play IR data(multiple files)\n"
"   -v - show firmware version\n"
"   -d - Debug\n"
"\n";

static bool showHelp = false;
static bool captureIR = false;
static bool playIR   = false;
static bool saveFile = false;
static bool showVersion = false;
static const char *jsonFile = NULL;
static bool showDebug = false;
static const char *inputNumber = NULL;

static int parseArgs(int argc, char * const argv[]) {
	int opt;
	while((opt = getopt(argc, argv, "hcspf:n:vd")) != -1) {//オプションがある引数には:(コロン)を後ろにつける
		switch(opt) {
			case 'h':
				showHelp = true;
				break;
			case 'c':
				captureIR = true;
				break;
			case 's':
				saveFile = true;
				break;
			case 'p':
				playIR = true;
				break;
			case 'f':
				jsonFile = optarg;
				break;
			case 'n':
				inputNumber = optarg;
				break;
			case 'v':
				showVersion = true;
				break;
			case 'd':
				showDebug = true;
				break;
		}
	}
	return optind;
}

int main(int argc, char *argv[])
{
	parseArgs(argc, argv);
	
	IR irm(showDebug);
	
	if(showHelp){
		printf(usage, argv[0]);
		return 0;
	}
	
	if(showVersion){
		irm.showversion();
		return 0;
	}

	if(captureIR){
		irm.capture();
		return 0;
	}

	if(saveFile &&jsonFile){
		irm.savefile(jsonFile);
		return 0;
	}
	
	if(playIR){
		if(jsonFile){
			irm.loadfile(jsonFile);
		}
		irm.play();
		return 0;
	}

	if(inputNumber){
		irm.playnumber(inputNumber);
		return 0;
	}
	
	return 0;
}