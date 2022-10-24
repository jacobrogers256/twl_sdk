/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - buryarg.TWL
  File:     buryarg.c

  Copyright 2008-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-17#$
  $Rev: 10762 $
  $Author: mizutani_nakaba $
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <getopt.h>

#include <nitro_win32.h>

//---- Error code
#define ERROR_OVER_AREA  		                0
#define ERROR_NO_FILE     		                1
#define ERROR_CANNOT_OPEN_FILE                  2
#define ERROR_CANNOT_OPEN_REP_FILE              3
#define ERROR_MARK_NOT_FOUND	                4
#define ERROR_BAD_MARK_POSITION	                5
#define ERROR_BAD_ARGUMENT_SIZE                 6
#define ERROR_UNKNOWN_OPTION                    7
#define ERROR_OUTFILE_NOT_SPECIFIED             8
#define ERROR_REPLACEMENTFILE_NOT_SPECIFIED   	9
#define ERROR_REPLACEMENTFILE_TOO_BIG           10
#define ERROR_ILLEGAL_OPTIONS                   11
#define ERROR_FILE_ERROR                        12
#define ERROR_NLF_FILE_SPECIFIED                13
#define ERROR_NOT_TWL_SRL                       14
#define ERROR_ILLEGAL_TLF_FILE					15

//---- Version
#define VERSION_STRING     " 1.2  Copyright 2008 Nintendo. All right reserved."
// Version 1.2: Changed argument data position to same position as buryarg
// Version 1.1: Fixed bug on --stdout option
// Version 1.0: Applied for TWL

//---- Output default added string
#define ADDED_OUTPUT_NAME  ".argAdded"

//---- Output for stdout
const char gNameForStdout[] = "-";

//---- Argument buffer identification string
const char gArgIdString[] = OS_ARGUMENT_ID_STRING;

//---- Input bin file
#define FILE_NAME_MAX_SIZE  1024
FILE   *gInputFile;
char    gInputFileNameString[FILE_NAME_MAX_SIZE];
BOOL    gIsInputFileOpened = FALSE;

//---- Output file
FILE   *gOutputFile;
char    gOutputFileNameString[FILE_NAME_MAX_SIZE];
char   *gOutputFileName = NULL;
BOOL    gIsOutputFileOpened = FALSE;

//---- Replacement file
FILE   *gReplacementFile;
char   *gReplacementFileName = NULL;
BOOL    gIsReplacementFileOpened = FALSE;

//---- TLF file
FILE   *gTlfFile;
char   *gTlfFileName = NULL;
BOOL    gIsTlfFileOpened = FALSE;
char    gTlfDirectoryNameString[FILE_NAME_MAX_SIZE];

//---- Argument buffer for replacement
#define ARGUMENT_BUF_SIZE 0x10000
char    gArgString[ARGUMENT_BUF_SIZE];
int     gArgStringSize;
int     gArgStringIndex = 0;

//---- Mode
BOOL    gQuietMode = FALSE;
BOOL    gVerboseMode = FALSE;
BOOL    gReplacementMode = TRUE;
BOOL    gDebugMode = FALSE;
BOOL    gTlfInputMode = FALSE;
BOOL    gStdoutMode = FALSE;

//---- Declaration of prototype
void    analyzeTlfFile(void);
void    closeFiles(void);

//================================================================================
//                      Message display
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         displayMessage

  Description:  Displays message.

  Arguments:    message: Message array

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayMessage(char *message[])
{
    int     n;
    //---- Show help messages
    for (n = 0; message[n]; n++)
    {
        printf(message[n]);
    }
}


/*---------------------------------------------------------------------------*
  Name:         displayUsage

  Description:  Displays how to use.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayUsage(void)
{
    char   *usageString[] = {
        "Usage: buryarg.TWL [OPTION]... <TWL srl filename> [argument]...\n",
        "Bury argument strings to TWL srl file.",
        "\n",
        "Options:\n",
        "  --version           : Show version.\n",
        "  -h, --help          : Show this help.\n",
        "  -q, --quiet         : Quiet mode.\n",
        "  -v, --verbose       : Verbose mode.\n",
        "  -r, --remain        : Remain original file.\n",
        "  -o, --output=FILE   : Output file (default: srcfile.argAdded)\n",
        "                        if \"-\" is specified, output to stdout.\n",
        "      --stdout        : Mean '-o-' .\n",
        "  -f, --file=FILE     : Specify replacement buffer data.\n",
        "                        This is used instead of data from arguments.\n",
        "  -d, --debug         : Debug mode. Show replacement buffer.\n",
        0
    };

    displayMessage(usageString);
}

/*---------------------------------------------------------------------------*
  Name:         displayVersion

  Description:  Displays version.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayVersion(void)
{
    printf("buryarg %s\n", VERSION_STRING);
}

/*---------------------------------------------------------------------------*
  Name:         displayError

  Description:  Displays error

  Arguments:    errorNumber: Error number

  Returns:      None.
 *---------------------------------------------------------------------------*/
void displayError(int errorNumber)
{
    fprintf(stderr, "Error: buryarg: ");

    switch (errorNumber)
    {
    case ERROR_OVER_AREA:
        fprintf(stderr, "arguments too long.\n");
        break;

    case ERROR_NO_FILE:
        fprintf(stderr, "cannot open binary file.\n");
        break;

    case ERROR_CANNOT_OPEN_FILE:
        fprintf(stderr, "cannot open output file.\n");
        break;

    case ERROR_CANNOT_OPEN_REP_FILE:
        fprintf(stderr, "cannot open replacement file.\n");
        break;

    case ERROR_MARK_NOT_FOUND:
        fprintf(stderr, "cannot replace because bin file has no argument id string.\n");
        break;

    case ERROR_BAD_MARK_POSITION:
        fprintf(stderr, "bad argument mark position.\n");
        break;

    case ERROR_BAD_ARGUMENT_SIZE:
        fprintf(stderr, "bad argument size.\n");
        break;

    case ERROR_UNKNOWN_OPTION:
        fprintf(stderr, "unknown option or invalid usage. try --help.\n");
        break;

    case ERROR_OUTFILE_NOT_SPECIFIED:
        fprintf(stderr, "output file is not specified.\n");
        break;

    case ERROR_REPLACEMENTFILE_NOT_SPECIFIED:
        fprintf(stderr, "replacement file is not specified.\n");
        break;

    case ERROR_REPLACEMENTFILE_TOO_BIG:
        fprintf(stderr, "replacement file is too big.\n");
        break;

    case ERROR_ILLEGAL_OPTIONS:
        fprintf(stderr, "specified illegal option set.\n");
        break;

    case ERROR_FILE_ERROR:
        fprintf(stderr, "error occurred in renaming file.\n");
        break;

	case ERROR_NLF_FILE_SPECIFIED:
		fprintf(stderr, "cannot access to nlf file.\n");
		break;

	case ERROR_NOT_TWL_SRL:
        fprintf(stderr, "ROM is not for TWL.\n");
        break;

    case ERROR_ILLEGAL_TLF_FILE:
        fprintf(stderr, "illegal tlf file.\n");
        break;

    default:
        fprintf(stderr, "unknown error (%d).\n", errorNumber);
        break;
    }

    closeFiles();
    exit(2);
}


//================================================================================
//                      Display for debug
//================================================================================
//---- For debug
//     Display buffer
void displayBuffer(void)
{
    const int showSize = 256;
    int     n;

    for (n = 0; n < showSize; n++)
    {
        int     k = n % 16;
        int     c;

        if (k == 0)
        {
            printf("%4x ", n);
        }

        c = (int)gArgString[n];

        if (0x20 <= c && c <= 0x7f)
        {
            printf("%c  ", gArgString[n]);
        }
        else if (c == 0)
        {
            printf(".  ");
        }
        else
        {
            printf("-  ");
        }

        if (k == 15)
        {
            printf("\n");
        }
    }
}


//================================================================================
//              Parse filename
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         getTailFileName

  Description:  Searches for tail pointer of filename.
                Example: If d:/aaa/bbb.srl was input, outputs bbb.srl.

  Arguments:    fileName: Filename

  Returns:      Tail pointer of original string.
 *---------------------------------------------------------------------------*/
const char *getTailFileName(const char *fileName)
{
    const char *p = fileName;
    const char *tailPtr = p;

    while (*p)
    {
        if (*p == '\\' || *p == '/' || *p == ':')
        {
            tailPtr = (p + 1);
        }

        p++;
    }

    return tailPtr;
}

//================================================================================
//              Option parse and buffer creation
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         checkOverBuffer

  Description:  Checks if buffer is over.
				(If error occurred, never return.)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void checkOverBuffer(void)
{
    if (gArgStringIndex >= ARGUMENT_BUF_SIZE)
    {
        displayError(ERROR_OVER_AREA);
    }
}

/*---------------------------------------------------------------------------*
  Name:         addString

  Description:  Adds string to TWL argument string area.

  Arguments:    str: String to add

  Returns:      None.
 *---------------------------------------------------------------------------*/
void addString(const char *str)
{
    const char *p = str;

    if (!str)
    {
        printf("internal error: tend to add NULL string.\n");
        exit(2);
    }

    while (*p)
    {
        checkOverBuffer();
        gArgString[gArgStringIndex++] = *p++;
    }

    checkOverBuffer();
    gArgString[gArgStringIndex++] = '\0';
}

/*---------------------------------------------------------------------------*
  Name:         checkTwlHeader

  Description:  Checks if the specified ROM is for TWL.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define ROM_TYPE_OFS 12
void checkTwlHeader(void)
{
	const int readSize=32;
   	char romHeader[readSize];
	int  romType;   

    fseek(gTlfFile, 0, SEEK_SET);
	size_t  size = fread(romHeader, 1, readSize, gTlfFile);

	romType = (int)romHeader[ ROM_TYPE_OFS ];

	if (size != readSize || !(romType & 0x2) )
	{
        displayError(ERROR_NOT_TWL_SRL);
	}
}

/*---------------------------------------------------------------------------*
  Name:         parseOption

  Description:  Parses the option line.

  Arguments:    argc: Argument count
                argv: Argument vector

  Returns:      Result. Less than 0 if error.
 *---------------------------------------------------------------------------*/
void parseOption(int argc, char *argv[])
{
    int     n;
    BOOL    helpFlag = FALSE;

    int     c;

    struct option optionInfo[] = {
        {"help", no_argument, NULL, 'h'},
        {"quiet", no_argument, NULL, 'q'},
        {"verbose", no_argument, NULL, 'v'},
        {"replacement", no_argument, NULL, 'r'},
        {"version", no_argument, NULL, 'a'},
        {"debug", no_argument, NULL, 'd'},
        {"output", required_argument, 0, 'o'},
        {"stdout", no_argument, NULL, 'O'},
        {"file", required_argument, 0, 'f'},
        {NULL, 0, 0, 0}
    };
    int     optionIndex;

    //---- Suppress error string of getopt_long()
    opterr = 0;

    while (1)
    {
        c = getopt_long(argc, argv, "+hqvrdo:f:", &optionInfo[0], &optionIndex);

        //printf("optind=%d optopt=%d  %x(%c) \n", optind, optopt, c,c );

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 'h':
            helpFlag = TRUE;
            break;
        case 'q':
            gQuietMode = TRUE;
            break;
        case 'v':
            gVerboseMode = TRUE;
            break;
        case 'r':
            gReplacementMode = FALSE;
            break;
        case 'd':
            gDebugMode = TRUE;
            break;
        case 'a':
            displayVersion();
            exit(1);
            break;
        case 'o':
            gOutputFileName = (*optarg == '=') ? optarg + 1 : optarg;
            if (!*gOutputFileName)
            {
                displayError(ERROR_OUTFILE_NOT_SPECIFIED);
            }
            gReplacementMode = FALSE;
            break;
        case 'O': 
            gOutputFileName = (char *)gNameForStdout;
            gReplacementMode = FALSE;
            break;
        case 'f':
            gReplacementFileName = (*optarg == '=') ? optarg + 1 : optarg;
            if (!*gReplacementFileName)
            {
                displayError(ERROR_REPLACEMENTFILE_NOT_SPECIFIED);
            }
            break;
        default:
            displayError(ERROR_UNKNOWN_OPTION);
        }
    }

    //---- No specified bin file or "-h" of "--help" to display usage
    if (helpFlag || argc <= optind)
    {
        displayUsage();
        exit(1);
    }

    //---- Input filename
    strncpy(gInputFileNameString, argv[optind], FILE_NAME_MAX_SIZE);

    //---- Check if TLF file
    if ((gTlfFile = fopen(gInputFileNameString, "rb")) == NULL)
    {
        displayError(ERROR_NO_FILE);
    }
    gIsTlfFileOpened = TRUE;

    //---- Check NLF and TLF ID string
    {
        char    idString[4];
        size_t  size = fread(idString, 1, 4, gTlfFile);
        if (size == 4 && !strncmp(idString, (char *)"#NLF", 4))
        {
			displayError(ERROR_NLF_FILE_SPECIFIED);
        }

		if (size == 4 && !strncmp(idString, (char *)"#TLF", 4))
		{
            gTlfFileName = argv[optind];

            //---- Determine input filename
            analyzeTlfFile();
		}
    }

	//---- Check if TWL header
	if (gTlfFileName == NULL)
	{
		checkTwlHeader();
	}

    //---- Close file
    fclose(gTlfFile);
    gIsTlfFileOpened = FALSE;

    //---- Add string to replacement buffer
    for (n = optind; n < argc; n++)
    {
        if (n == optind)
        {
            if (!gTlfFileName)
            {
                addString(getTailFileName(argv[n]));
            }
        }
        else
        {
            addString(argv[n]);
        }
    }

    //---- End mark
    checkOverBuffer();
    gArgString[gArgStringIndex++] = '\0';
}

//================================================================================
//                      File operations
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         analyzeTlfFile

  Description:  Analyzes TLF file.
 
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void analyzeTlfFile(void)
{
    char    lineBuffer[1024];
    BOOL    isFound_T = FALSE;
    BOOL    isFound_9 = FALSE;

    fseek(gTlfFile, 0, SEEK_SET);

    //---- Analyze
    while (fgets(lineBuffer, 1024, gTlfFile))
    {
        if (!strncmp(lineBuffer, "T,", 2))
        {
            char   *sp = &lineBuffer[0];
            char   *dp = &gTlfDirectoryNameString[0];

            while (*sp++ != '\"')
            {
            }
            while (*sp != '\"')
            {
                *dp++ = *sp++;
            }
            *dp = '\0';

            isFound_T = TRUE;
        }

		if (!strncmp(lineBuffer, "9,", 2) && isFound_T)
		{
			char *sp = &lineBuffer[0];
			char *dp = &gInputFileNameString[0];
			int count=0;

			while (*sp && count <=2)
			{
				if (*sp++ == '\"')
				{
					count++;
				}
			}
			while (*sp != '\"')
			{
				*dp++ = *sp++;
			}
			*dp = '\0';

			//---- Set argv[0]
			addString(getTailFileName(gInputFileNameString));

			isFound_9 = TRUE;
		}
    }

    //---- Check if found filename
    if (!isFound_9)
    {
        displayError(ERROR_ILLEGAL_TLF_FILE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         openFiles

  Description:  Opens original file and output file.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void openFiles(void)
{
    //---- Replacement file
    if (gReplacementFileName)
    {
        if (gVerboseMode)
        {
            printf("replacement file    : %s\n", gReplacementFileName);
        }
        if ((gReplacementFile = fopen(gReplacementFileName, "rb")) == NULL)
        {
            displayError(ERROR_CANNOT_OPEN_REP_FILE);
        }
        gIsReplacementFileOpened = TRUE;

        //---- Replace buffer
        {
            int n;
            int c;

            if (gVerboseMode)
            {
                struct stat repFileStat;
                stat(gReplacementFileName, &repFileStat);
                printf("replacement file size is 0x%x byte (%d byte).\n", (int)repFileStat.st_size,
                       (int)repFileStat.st_size);
            }

            //---- Clear buffer
            for (n = 0; n < ARGUMENT_BUF_SIZE; n++)
            {
                gArgString[n] = 0;
            }

            //---- Get replacement strings
            n = 0;
            while (n < ARGUMENT_BUF_SIZE && (c = fgetc(gReplacementFile)) != EOF)
            {
                gArgString[n] = c;
                n++;
            }
            fclose(gReplacementFile);
            gIsReplacementFileOpened = FALSE;

            //---- Check over buffer
            if (c != EOF)
            {
                //displayError( ERROR_REPLACEMENTFILE_TOO_BIG );
                if (!gQuietMode)
                {
                    printf("warning: replacement file is bigger than required size.");
                }
            }
        }
    }

    //---- Do not overwrite original file
    if (!gReplacementMode)
    {
        //---- Written filename
        if (gOutputFileName)
        {
            strncpy(gOutputFileNameString, gOutputFileName, FILE_NAME_MAX_SIZE);
        }
        else
        {
            strcat(gOutputFileNameString, gInputFileNameString);
            strncat(gOutputFileNameString, ADDED_OUTPUT_NAME, FILE_NAME_MAX_SIZE);
        }

        //---- Check if stdout specified
        if (!strncmp(gOutputFileNameString, "-", 1))
        {
            gStdoutMode = TRUE;
        }

        //---- Output file
        if (gVerboseMode)
        {
            if (gStdoutMode)
            {
                printf("destination         : stdout\n");
            }
            else
            {
                printf("destination bin file: %s\n", gOutputFileNameString);
            }
        }
        if (gStdoutMode)
        {
            gOutputFile = stdout;

            //---- Set stdout to binary mode
            _setmode(_fileno(gOutputFile), O_BINARY);
        }
        else
        {
            if ((gOutputFile = fopen(gOutputFileNameString, "wb")) == NULL)
            {
                displayError(ERROR_CANNOT_OPEN_FILE); 
           }
            gIsOutputFileOpened = TRUE;
        }
    }

    //---- Input file
    if (gTlfFileName)
    {
        if (gVerboseMode)
        {
            printf("tlf current directory is %s\n", gTlfDirectoryNameString);
        }
        //---- Change current directory
        if (chdir(gTlfDirectoryNameString) != 0)
        {
            displayError(ERROR_ILLEGAL_TLF_FILE);
        }
    }


    if (gVerboseMode)
    {
        printf("original bin file   : %s\n", gInputFileNameString);
    }
    if ((gInputFile = fopen(gInputFileNameString, (gReplacementMode) ? "rb+" : "rb")) == NULL)
    {
        displayError(ERROR_NO_FILE);
    }
    gIsInputFileOpened = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         closeFiles

  Description:  Closes files.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void closeFiles(void)
{
    if (gStdoutMode)
    {
        //---- Flush stdout
        fflush(stdout);

        //---- Set stdout to text mode
        _setmode(_fileno(gOutputFile), O_TEXT);
    }

    if (gIsInputFileOpened)
    {
        fclose(gInputFile);
        gIsInputFileOpened = FALSE;
    }
    if (gIsOutputFileOpened)
    {
        fclose(gOutputFile);
        gIsOutputFileOpened = FALSE;
    }
    if (gIsReplacementFileOpened)
    {
        fclose(gReplacementFile);
        gIsReplacementFileOpened = FALSE;
    }
    if (gIsTlfFileOpened)
    {
        fclose(gTlfFile);
        gIsTlfFileOpened = FALSE;
    }
}

//================================================================================
//                      Replace buffer
//================================================================================
void replaceToSpecifiedString(void)
{
    BOOL    isFindMark = FALSE;
    int     markIndex = 0;
    long    bufPosition;
    int     n;
    int     inputChar;

    const int markLength = strlen(gArgIdString);

    //---- Search argument identification string in bin file
    while (1)
    {
        if ((inputChar = fgetc(gInputFile)) == EOF)
        {
            break;
        }

        if (inputChar == gArgIdString[markIndex])
        {
            markIndex++;
            if (markIndex == markLength)
            {
                long    topPosition = ftell(gInputFile) - markLength;
                bufPosition = topPosition + OS_ARGUMENT_ID_STRING_BUFFER_SIZE + sizeof(u16);

                if (gVerboseMode)
                {
                    printf("found argument identification string at 0x%x byte from top.\n",
                           (int)topPosition);
                }
                isFindMark = TRUE;
                break;
            }
        }
        else
        {
            if (markIndex > 0)
            {
                markIndex = 0;
                if (inputChar == gArgIdString[0])
                {
                    markIndex++;
                }
            }
        }
    }

    //---- Check if mark not found
    if (!isFindMark)
    {
        displayError(ERROR_MARK_NOT_FOUND);
    }

    //---- Position check
    if (bufPosition < 0)
    {
        displayError(ERROR_BAD_MARK_POSITION);
    }

    //---- Get buffer size
    fseek(gInputFile, bufPosition - sizeof(u16), SEEK_SET);
    gArgStringSize = (int)(fgetc(gInputFile) + (fgetc(gInputFile) << 8));

    if (gVerboseMode)
    {
        printf("buffer size = 0x%x byte\n", gArgStringSize);
    }

    if (gArgStringSize < 0 || gArgStringSize > ARGUMENT_BUF_SIZE)
    {
        displayError(ERROR_BAD_ARGUMENT_SIZE);
    }

    //---- Put forced endmark to replace buffer
    gArgString[gArgStringSize - 2] = '\0';
    gArgString[gArgStringSize - 1] = '\0';

    //---- Replacement
    if (gReplacementMode)
    {
        fseek(gInputFile, bufPosition, SEEK_SET);
        for (n = 0; n < gArgStringSize; n++)
        {
            fputc(gArgString[n], gInputFile);
        }

        if (gVerboseMode)
        {
            printf("replaced 0x%x byte.\n", (int)gArgStringSize);
        }
    }
    else
    {
        fseek(gInputFile, 0, SEEK_SET);
        n = 0;
        while ((inputChar = fgetc(gInputFile)) != EOF)
        {
            if (bufPosition <= n && n < bufPosition + gArgStringSize)
            {
                fputc(gArgString[n - bufPosition], gOutputFile);
            }
            else
            {
                fputc(inputChar, gOutputFile);
            }

            n++;
        }

        if (gVerboseMode)
        {
            printf("output 0x%x byte (%d byte).\n", n, n);
        }
    }
}

//================================================================================
//                      Main routine
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         Main

  Description:  Main proc.

  Arguments:    argc: Argument count
                argv: Argument vector

  Returns:      ---
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    //---- Option parsing
    parseOption(argc, argv);

    //---- File open
    openFiles();

    //---- Buffer internal display (for debugging)
    if (gDebugMode)
    {
        displayBuffer();
    }

    //---- Replacement processing
    replaceToSpecifiedString();

    //---- File close
    closeFiles();

    //---- End
    if (!gQuietMode)
    {
        printf("Success.\n");
    }
    return 0;
}
