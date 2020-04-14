#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>

#define PATHLEN 5	//subdirectory max
#define PATHMAX 128	//space for path
#define BUFFSIZE 256	//buffer space
// UNDONE: function only copies in same directory
// TODO: copy to different location(aka move)

/*	Function copies contents of a file to a specified destination
*	arguments: sourceFile(file to be copied) and destinationFile(file contents are copied to
*	returns 0 if failed, 1 if successful
*/
int copyMove(char* sourceFile, char* destinationFile)
{
	FILE* sourceFilePtr,		//pointer for sourcefile
		* destinationFilePtr;	//pointer for destination file
	char letter,	//letter to be copied
		* fileName = calloc(BUFFSIZE, sizeof(char*)),	//holds filename
		* path = calloc(BUFFSIZE, sizeof(char*)),		//holds path
		* sourceArr = calloc(PATHLEN, sizeof(char*)),			//array to hold path for source file
		* destinationArr = calloc(PATHLEN, sizeof(char*));	//array to hold path for destination file


	if (sourceArr == NULL)
		printf("Could not allocate sourceArr\n");
	if(destinationArr == NULL)
		printf("could not allocate destinationArr\n");

	//parse path and file name
	char* token = strtok(sourceFile, "\\/\n"); //starts tokenizing
	int pathLevel = 0;	//count for pathing purpose
	while (token != NULL)	//loop to grab rest of the string(path)
	{
		printf("%s\n", token);
		//sourceArr[pathLevel] = token;
		strcpy(sourceArr[pathLevel], token);
		token = strtok(NULL, "\\/");
		if (token != NULL)
			pathLevel++;
	}
	strcpy(fileName, sourceArr[pathLevel]);

	if (pathLevel == 0)
	{
		sourceFilePtr = fopen(fileName, "r"); //opens the source file
		if (sourceFilePtr == NULL)	//if source file couldn't be opened
		{
			printf("File \"%s\" not found.\n", sourceFile);	//print statement
			return 0;		//return 0 for false
		}
	}
	else
	{
		strcpy(path, sourceArr[0]);
		for (int i = 1; i < pathLevel; i++)
		{

		}
		//chdir()
	}

	/*
	token = strtok(destinationFile, "\\/");
	count = 0;
	while (token != NULL)
	{
		strcpy(destinationArr[count], token);
		count++;
		token = strtok(NULL, "\\/");
	}
	strcpy(sourceFile, destinationArr[count);
	*/

	if (pathLevel == 0)
	{
		destinationFilePtr = fopen(destinationFile, "w");	//opens destination file
		if (destinationFilePtr == NULL)		//if destination file couldn't be opened
		{
			printf("Destination file \"%s\" can't be written to.\n", destinationFile);	//print statement
			return 0;	//return 0 for false
		}
	}
	else
	{

	}

	letter = fgetc(sourceFilePtr);	//copies first letter from source file
	//while loop to copy each letter to the destination file
	while (letter != EOF)	//while letter is not at the end of the file
	{
		fputc(letter, destinationFilePtr);	//copies letter into the destination file
		letter = fgetc(sourceFilePtr);	//gets next letter from source file
	}

	printf("Contents from \"%s\" copied to \"%s\".\n", sourceFile, destinationFile);	//print statement
	free(sourceArr);
	free(destinationArr);
	fclose(sourceFilePtr);	//close source file
	fclose(destinationFilePtr);	//close destination file
	return 1;	//return 1 for a successful copy
}
