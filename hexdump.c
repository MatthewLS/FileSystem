/****************************************************************************
 *	hexdump - a program to display a file in hexadecimal and ascii
 *	Such as:
 *		000020: 19 00 00 00 48 00 00 00  5F 5F 50 41 47 45 5A 45 | ....H...__PAGEZE
 *
 *  Author:		Robert Bierman
 *	Date:		March 28, 2020
 *	Source:		hexdump.c
 *
 *  Primary Purpose:
 *	Written as a utility for CSC-415 Operating Systems, File System project
 *  to allow the dumping of the "drive" file for verification of proper 
 *  structure and content.
 *
 *  copyright 2020 Robert Bierman
 ****************************************************************************/

// Compilation:	gcc hexdump.c -o hexdump

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


#define BUFSIZE 4096		//amount read at one time
#define BLOCKSIZE 256		// number of bytes printed before a blank line
#define LBABLOCKSIZE	512	// display blocks from command line are based on 512 bytes
#define VERSION "1.0"		// Version

// This procedure takes a file name, a starting block and a number of blocks and dumps the
// file to stdout.  
// Output (including the number of blocks) is limited by the length of the file and
// partial blocks are counted as a whole block for the header showing how many blocks are
// being displayed.
//
// Checks are done to ensure that blanks are displayed on the last line of the output
// if the file only partially uses the last 16 bytes.

int processFile (char * filename, uint64_t startBlock, uint64_t numBlocks)
	{
	int readbytes;
	int position = 0;
	int loops = BUFSIZE / BLOCKSIZE;	//number of loops of blocks within one buffer read
	int offset;
	int k;
	uint32_t lbaBlockSize = LBABLOCKSIZE;
	uint64_t numBytesToStartBlock;
	uint64_t numBytesToProcess;
	uint64_t endOfFile;

	numBytesToProcess = numBlocks * lbaBlockSize;
	numBytesToStartBlock = startBlock * lbaBlockSize;
	
	int fd = open (filename, O_RDONLY);			//open the file
	
	// Error opening file (common if they don't enter a valid file name)
	if (fd == -1)
		{
		printf ("ERROR: failed to open file '%s'\n", filename);
		return -2;
		}
		
	endOfFile = lseek(fd, 0, SEEK_END);  //will reset seek below
	
	if (numBytesToProcess == 0)
		{
		numBytesToProcess = endOfFile;
		//reset numBlocks for the header here
		numBlocks = ((numBytesToProcess + lbaBlockSize) - 1) / lbaBlockSize;
		numBlocks = numBlocks - startBlock;
		}

	unsigned char * buf = malloc (BUFSIZE);		//Allocate the read buffer
	
	// Very rare error - something bad if I can not allocate a small buffer
	if (buf == NULL)
		{
		close (fd);
		printf ("Failed to allocate buffer\n");
		return -3;
		}
	
	//Position to the startBlock
	lseek (fd, numBytesToStartBlock, SEEK_SET);
	position = numBytesToStartBlock;
	
	if (position > endOfFile)		//can not start past the end of the filename
		{
		printf ("Can not dump file %s, starting at block %llu, past the end of the file.\n\n", 
			filename, (unsigned long long)startBlock);	
		return (-5);
		}

	// calculate max blocks we can display from the given start point
	uint64_t maxBlocks = (((endOfFile - position) + lbaBlockSize) - 1) / lbaBlockSize;
	if (numBlocks > maxBlocks)
		numBlocks = maxBlocks;

	
	//Proces the file - the do loop goes until we read less bytes than the BUFSIZE
	printf ("Dumping file %s, starting at block %llu for %llu block%c:\n\n", 
		filename, (unsigned long long)startBlock, (unsigned long long)numBlocks, numBlocks != 1?'s':'\0');	
	do
		{
		if (position >= (numBytesToStartBlock + numBytesToProcess))
			goto cleanup;
			
		readbytes = read (fd, buf, BUFSIZE);		//Read one block
		offset = 0;									//set our offset within the block
		for (int i = 0; i < loops; i++)				//Loop for each "Block" within one buffer read
			{
			for (int j = 0; j < BLOCKSIZE/16; j++)	//loop j lines for each block
				{
				if (position+offset >= (numBytesToStartBlock + numBytesToProcess))
					goto cleanup;
				
				// Handle if we are at the end of the file and the line will have less 
				// than 16 bytes associated with it.
				if (offset + 16 > readbytes)
					{
					printf ("%06X: ", offset+position);
					for (k = 0; k < readbytes - offset; k++)
						{
						printf ("%02X ", buf[offset + k]);
						}
					for (;k < 16; k++)	
						{
						printf ("   ");   //Print remaining of the hex output as blanks to fill out the line
						}
					printf (" | ");
					
					for (k = 0; k < readbytes - offset; k++)
						{
						printf ("%c", buf[offset + k] < 32?'.':buf[offset+k]);
						}
					printf("\n");
					
					}
				else	
					{
					//If a full line, do one print for the full line
					printf ("%06X: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X | %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
						offset+position,
						buf[offset + 0],buf[offset + 1],buf[offset + 2],buf[offset + 3],
						buf[offset + 4],buf[offset + 5],buf[offset + 6],buf[offset + 7],
						buf[offset + 8],buf[offset + 9],buf[offset + 10],buf[offset + 11],
						buf[offset + 12],buf[offset + 13],buf[offset + 14],buf[offset + 15],
						buf[offset + 0] < 32?'.':buf[offset + 0],
						buf[offset + 1] < 32?'.':buf[offset + 1],
						buf[offset + 2] < 32?'.':buf[offset + 2],
						buf[offset + 3] < 32?'.':buf[offset + 3],
						buf[offset + 4] < 32?'.':buf[offset + 4],
						buf[offset + 5] < 32?'.':buf[offset + 5],
						buf[offset + 6] < 32?'.':buf[offset + 6],
						buf[offset + 7] < 32?'.':buf[offset + 7],
						buf[offset + 8] < 32?'.':buf[offset + 8],
						buf[offset + 9] < 32?'.':buf[offset + 9],
						buf[offset + 10] < 32?'.':buf[offset + 10],
						buf[offset + 11] < 32?'.':buf[offset + 11],
						buf[offset + 12] < 32?'.':buf[offset + 12],
						buf[offset + 13] < 32?'.':buf[offset + 13],
						buf[offset + 14] < 32?'.':buf[offset + 14],
						buf[offset + 15] < 32?'.':buf[offset + 15]);
					}
				//up the offset by 16 for the next line	
				offset = offset + 16;
				
				//if greater than the readbytes we have exhausted this buffer
				if (offset >= readbytes)
					break;
					
				}
			//print a blank line between each BLOCK
			printf("\n");
			//if greater than the readbytes we have exhausted this buffer
			if (offset >= readbytes)
				break;
			}
		//Next buffer, increment the overall position within the file.	
		position = position + readbytes;
		
		// If we read the number of bytes requested (BUFSIZE), then we have not hit
		// the end of file yet, and should try to read more.
		} while (readbytes == BUFSIZE); 

cleanup:	
	// clean up	
	free (buf);
	close (fd);
	return 0;
	}

// processArguments handles the command line using getopt_long to parse the argv array
//
// It then sets variable and or dispatches the action necessary based on the parameters.
// it handles the multiple calls to processFile by iterating on uncontained parameters
int processArguments (int argc, char * argv[])
	{
	int c;
	int digit_optind = 0;
	uint64_t count, start;
	count = 0; start = 0;
	int retval;
	char * filename = NULL;
	
	while (1) 
		{
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		
		static struct option long_options[] = {
		   {"count",	required_argument, 0, 'c'},  //forces to c
		   {"start",	required_argument, 0, 's'},	 //forces to s
		   {"help",		no_argument,       0, 'h'},
		   {"version",	no_argument,       0, 'v'},
		   {"file",		required_argument, 0, 'f'},
		   {0,			0,                 0,  0 }
		};

		c = getopt_long(argc, argv, "c:s:f:vh",
				long_options, &option_index);
				
		if (c == -1)
		   break;

		switch (c) {
			case 0:			//It is a long option (all converted so should be none)
			
			   printf("Unknown option %s", long_options[option_index].name);
			   if (optarg)
				   printf(" with arg %s", optarg);
			   printf("\n");
			   exit (-1);

			case 'c':
			   count = atol(optarg);
			   break;

			case 's':
			   start = atol(optarg);
			   break;

			case 'f':
			   filename = optarg;
			   break;

			case 'h':
				printf ("USAGE: hexdump --file <filename> [--count num512ByteBlocks] [--start start512ByteBlock] [--help] [--version]\n");
			    exit (0);
			   
			case 'v':
				printf("hexdump - Version %s; copyright 2020 Robert Bierman\n\n", VERSION);
				exit (0);

			case '?':
			   break;

			default:
			   printf("Unknown option returned character code 0%o ??\n", c);
			   exit (-1);
			}
		}

	//if a file name is already specified - process it
	if (filename != NULL)
		{
		retval = processFile (filename, start, count);
		if (retval != 0)
			return (retval);	
		}
		
	//additional files (same arguments)		
	if (optind < argc) 
		{
        while (optind < argc)
			{
			retval = processFile (argv[optind++], start, count);	
			if (retval != 0)
				return (retval);	
			}
		}
		
	return 0;
	}


//Main calls process arguments which in turn calls process file.

int main (int argc, char * argv[])
	{
	return (processArguments (argc, argv));
	}
