#include "common.h"

const char *userFile = "<stdin>";
unsigned userLine;

const char *outputDir = NULL; // "output";

const char *inputFileName;
const char *outputFileName = "main.c";

const char *architecture = "telosb";

extern FILE *yyin;
extern FILE *yyout;

bool verboseMode;

// -----------------------------------

string toLowerCase(const string &s) {
    string result;
    result.reserve(s.size());
    foreach_const(string, s,
            result.push_back(tolower(*it)));
    return result;
}

string toUpperCase(const string &s) {
    string result;
    result.reserve(s.size());
    foreach_const(string, s,
            result.push_back(toupper(*it)));
    return result;
}

string toCamelCase(const string &s) {
    string result;
    result.reserve(s.size());
    foreach_const(string, s,
            if (it == s.begin()) {
                result.push_back(tolower(*it));
            } else {
                result.push_back(*it);
            });
    return result;
}

// -----------------------------------

void userError(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    // also print "file:line" for the offending line
    fprintf(stderr, "error in %s:%d: ", userFile, userLine);
    vfprintf(stderr, fmt, list);
    fputc('\n', stderr);
    va_end(list);
    exit(1);
}

void sysError(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    fprintf(stderr, "system error: ");
    vfprintf(stderr, fmt, list);
    fputc('\n', stderr);
    va_end(list);
    exit(1);
}

void help(bool error)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  -a <arch>, --arch    Architecture\n");
    fprintf(stderr, "  -o, --output [FILE]  Output to file (default: %s)\n",
            outputFileName);
    fprintf(stderr, "  -V, --verbose        Verbose mode\n");
    fprintf(stderr, "  -v, --version        Print version\n");
    fprintf(stderr, "  -h, --help           Print this help\n");
    exit(error ? EXIT_FAILURE : EXIT_SUCCESS);
}

void parseOptions(int argc, char *argv[])
{
    struct option longOptions[] = {
        {"arch", 1, 0, 'a'},
        {"help", 0, 0, 'h'},
        {"output", 1, 0, 'o'},
        {"verbose", 0, 0, 'V'},
        {"version", 0, 0, 'v'},
        {0, 0, 0, 0}
    };

    bool showHelp = false;
    bool isError = false;
    for (;;) {
        int index = 0;
        int c = getopt_long(argc, argv, "a:ho:Vv", longOptions, &index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'a':
            architecture = strdup(optarg);
            break;
        case 'v':
            printf ("Version: %s (built %s)\n", PROGRAM_VERSION, __DATE__);
            exit(0);
            break;
        case 'V':
            verboseMode = true;
            break;
        case 'h':
            showHelp = true;
            break;
        case 'o':
            outputFileName = strdup(optarg);
            break;
        case ':':
            fprintf(stderr, "Option -%c requires an operand\n", optopt);
            isError = true;
            break;
        case '?':
            fprintf(stderr, "Unrecognized option -%c\n", optopt);
            isError = true;
            break;
        }
    }

    if (showHelp || isError) {
        help(isError);
    }

    // take last, unnamed argument as input file name
    if (optind < argc) {
        inputFileName = argv[optind];
    }
}

int main(int argc, char *argv[])
{
    // read command line options
    parseOptions(argc, argv);

    // static initialization - all components for conditions
    initComponentMap();

    // parse input file
    if (inputFileName) {
        yyin = fopen(inputFileName, "r");
        if (!yyin) {
            sysError("Failed to open input file %s\n", inputFileName);
            exit(EXIT_FAILURE);
        }
    } else {
        yyin = stdin;
    }
    yyout = stdout;
    yyparse();

    if (outputDir && mkdir(outputDir, 0755) != 0) {
        if (errno != EEXIST) {
            sysError("Failed to create output directory %s\n", outputDir);
        }
    }

    // generate code
    if (verboseMode) printf("generating code...\n");
    generateCode();
    if (verboseMode) printf("generating config file...\n");
    generateConfigFile();
    // printf("generating Makefile...\n");
    // generateMakefile();
    // generateSerialReader(); // TODO
    // generateBaseStation();  // TODO
    if (verboseMode) printf("done!\n");
}
