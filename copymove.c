#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXPATHLEN 5	//subdirectory max
#define PATHMAX 128	//space for path
#define BUFFSIZE 256	//buffer space
// UNDONE: function only copies in same directory
// TODO: copy to different location(aka move)

int copyMove(char*, char*);

int main(int argc, char* argv[])
{
	char* sourceFile = argv[1];
	copyMove(sourceFile, "test2.txt");
	printf("main\n");
	//printf("test\n");
}


/*	Function copies contents of a file to a specified destination
*	arguments: sourceFile(file to be copied) and destinationFile(file contents are copied to
*	returns 0 if failed, 1 if successful
*/
int copyMove(char* sourceFile, char* destinationFile)
{
	FILE* sourceFilePtr,		//pointer for sourcefile
		* destinationFilePtr;	//pointer for destination file
	char letter,	//data to be copied
		* sourceFileName = calloc(BUFFSIZE, sizeof(char*)),	//holds filename
		* path = calloc(BUFFSIZE, sizeof(char*)),		//holds path
		** sourceArr = malloc((MAXPATHLEN) * sizeof(char)),			//array to hold path for source file
		** destinationArr = malloc((MAXPATHLEN) * sizeof(char));	//array to hold path for destination file
	int pathLevel = 0,	//count for pathing purpose
		sourceArrSize;	//size of source array

	//check if space could not be allocated
	if (sourceArr == NULL)
		printf("Could not allocate sourceArr\n");
	if(destinationArr == NULL)
		printf("could not allocate destinationArr\n");

	//for loop to allocate space per each individual item
	for(int i = 0; i < MAXPATHLEN; i++)
	{
		sourceArr[i] = malloc(BUFFSIZE * sizeof(char));
		if(sourceArr[i] == NULL)
			printf("Could not allocate space.\n");
	}

	//parse path and file name
	char* token = strtok(sourceFile, "/\\\n"); //starts tokenizing
	
	while (token != NULL)	//loop to grab rest of the string(path)
	{
		printf("%s\n", token);
		strcpy(sourceArr[pathLevel], token);
		token = strtok(NULL, "/\\\n");
		if (token != NULL){
			pathLevel++;
			sourceArrSize++;
		}
	}
	printf("pathlevel:%i\n",pathLevel);
	strcpy(sourceFileName, sourceArr[pathLevel]);

	if (pathLevel == 0)
	{
		printf("in current directory\n");
		sourceFilePtr = fopen(sourceFileName, "r"); //opens the source file
		if (sourceFilePtr == NULL)	//if source file couldn't be opened
		{
			printf("File \"%s\" not found.\n", sourceFile);	//print statement
			return 0;		//return 0 for false
		}
	}
	else
	{
		printf("in directories\n");
		/*
		strcpy(path, sourceArr[0]);
		for (int i = 1; i < pathLevel; i++)
		{

		}
		//chdir()
		*/
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
/*
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
	*/
	printf("Contents from \"%s\" copied to \"%s\".\n", sourceFileName, destinationFile);	//print statement
	printf("sourcesize:%i\n", sourceArrSize);
	for(int i = 0; i <= MAXPATHLEN ; i++)
	{
		free(sourceArr[i]);
	}
	free(sourceArr);
//	free(destinationArr);
//	fclose(sourceFilePtr);	//close source file
	//fclose(destinationFilePtr);	//close destination file
	return 1;	//return 1 for a successful copy
}
