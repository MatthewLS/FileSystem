#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXPATHLEN 5	//subdirectory max
#define PATHMAX 256		//space for path
#define FILENAMESIZE 32
#define BUFFSIZE 20	//buffer space
// UNDONE: function only copies in same directory
// TODO: copy to different location(aka move)

int copy(char*, char*);

int main(int argc, char* argv[])
{
	char* sourceFile = argv[1],
		* destinationFile = argv[2];
	copy(sourceFile, destinationFile);
	printf("back in main\n");
}


/*	Function copies contents of a file to a specified destination
*	arguments: sourceFile(file to be copied) and destinationFile(file contents are copied to
*	returns 0 if failed, 1 if successful
*/
int copy(char* sourceFile, char* destination)
{
	FILE* sourceFilePtr,		//pointer for sourcefile
		* destinationFilePtr;	//pointer for destination file
	char letter,	//data to be copied
		* fileContents = malloc(BUFFSIZE * sizeof(char)),
		* sourceFileName = malloc(FILENAMESIZE * sizeof(char)),	//holds filename
		* destinationFileName = malloc(FILENAMESIZE * sizeof(char)), //holds destination name
		* sourcePath = malloc(PATHMAX * sizeof(char)),				//holds source path
		* destinationPath = malloc(PATHMAX * sizeof(char)),		//holds destination path
		** sourceArr = malloc((MAXPATHLEN) * sizeof(char)),			//array to hold path for source file
		** destinationArr = malloc((MAXPATHLEN) * sizeof(char));	//array to hold path for destination file
	int pathLevel = 0,	//count for pathing purpose
		sourceArrSize,	//size of source array
		destinationArrSize, //size of destination array
		directoryChanged,	//return value for chdir function
		contentCounter = 0;

	//check if space could not be allocated
	if (sourceArr == NULL)
		printf("Could not allocate sourceArr\n");
	if(destinationArr == NULL)
		printf("could not allocate destinationArr\n");

	//for loop to allocate space per each individual item
	for(int i = 0; i < MAXPATHLEN; i++)
	{
		sourceArr[i] = malloc(FILENAMESIZE * sizeof(char));
		if(sourceArr[i] == NULL)
			printf("Could not allocate space.\n");
	}

	//parse path and file name
	printf("\tSource file\n");
	char* token = strtok(sourceFile, "/\\\n"); //starts tokenizing
	
	while (token != NULL)	//loop to grab rest of the string(path)
	{
		printf("%s\n", token);
		strcpy(sourceArr[pathLevel], token);	//copies token into source array
		token = strtok(NULL, "/\\\n");	//gets next token
		if (token != NULL) //if not at the last token
		{
			pathLevel++;		//increase path level counter
			sourceArrSize++;	//increase array counter
		}
	}
	printf("pathlevel:%i\n",pathLevel);
	strcpy(sourceFileName, sourceArr[pathLevel]);	//pathlevel should be the file name, copies into variable sourceFileName

	if (pathLevel == 0)	//case for current working directory
	{
		printf("in current directory\n");
		sourceFilePtr = fopen(sourceFileName, "r"); //opens the source file
		if (sourceFilePtr == NULL)	//if source file couldn't be opened
		{
			printf("File \"%s\" not found.\n", sourceFile);	//print statement
			return 0;		//return 0 for false
		}
	}
	else	//case for a path given
	{
		printf("in directories\n");
		strcpy(sourcePath, sourceArr[0]);	//gets initial path
		//loop to piece the whole path together
		for (int i = 1; i < pathLevel; i++)
		{
			strcat(sourcePath, "/");
			strcat(sourcePath, sourceArr[i]);
		}
		printf("%s\n",sourcePath);
		directoryChanged = chdir(sourcePath);	//changes to directory where file is located
		if(directoryChanged == 0)	//if successful
		{
			printf("directory changed.\n");
			sourceFilePtr = fopen(sourceFileName, "r"); //opens the source file
			if (sourceFilePtr == NULL)	//if source file couldn't be opened
			{
				printf("File \"%s\" not found.\n", sourceFileName);	//print statement
				return 0;		//return 0 for false
			}
			else
				printf("File opened.\n");
		}
		else	//not successful directory change
		{
			printf("directory change failed.\n");
			return 0;
		}
	}

	
	//copy
	printf("path\n");
	printf("\n\n\tCopying file\n");
	letter = fgetc(sourceFilePtr);	//copies first letter from source file
	strcpy(fileContents[0], letter);
	//while loop to copy each letter to the destination file
	while (letter != EOF)	//while letter is not at the end of the file
	{	
		printf("letters:%c ", letter);
		if(contentCounter >= BUFFSIZE )
		{
			printf("need to realloc\n");
			return 0;
		}
		//fputc(letter, destinationFilePtr);	//copies letter into the destination file
		letter = fgetc(sourceFilePtr);	//gets next letter from source file
	}

	//destination file
	printf("\n\n\tDestination file\n");
	//for loop to allocate space per each individual item
	for(int i = 0; i < MAXPATHLEN; i++)
	{
		destinationArr[i] = malloc(FILENAMESIZE * sizeof(char));
		if(destinationArr[i] == NULL)
			printf("Could not allocate space.\n");
	}
	
	token = strtok(destination, "/\\\n"); //starts tokenizing
	pathLevel = 0;

	while (token != NULL)	//loop to grab rest of the string(path)
	{
		printf("%s\n", token);
		strcpy(destinationArr[pathLevel], token);	//copies token into source array
		token = strtok(NULL, "/\\\n");	//gets next token
		if (token != NULL) //if not at the last token
		{
			pathLevel++;		//increase path level counter
			destinationArrSize++;	//increase array counter
		}
	}
	printf("pathlevel:%i\n",pathLevel);
	strcpy(destinationFileName, destinationArr[pathLevel]);	//pathlevel should be the file name, copies into variable sourceFileName

	if (pathLevel == 0)	//case for current working directory
	{
		printf("in current directory\n");
		destinationFilePtr = fopen(destinationFileName, "w"); //opens the source file
		if (destinationFilePtr == NULL)	//if source file couldn't be opened
		{
			printf("File \"%s\" not found.\n", destinationFileName);	//print statement
			//return 0;		//return 0 for false
		}
	}
	else	//case for a path given
	{
		printf("in directories\n");
		strcpy(destinationPath, destinationArr[0]);	//gets initial path
		//loop to piece the whole path together
		for (int i = 1; i < pathLevel; i++)
		{
			strcat(destinationPath, "/");
			strcat(destinationPath, destinationArr[i]);
		}
		printf("%s\n",destinationPath);
		directoryChanged = chdir(destinationPath);	//changes to directory where file is located
		if(directoryChanged == 0)	//if successful
		{
			printf("directory changed.\n");
			sourceFilePtr = fopen(destinationFileName, "w"); //opens the source file
			if (sourceFilePtr == NULL)	//if source file couldn't be opened
			{
				printf("File \"%s\" not found.\n", destinationFileName);	//print statement
				//return 0;		//return 0 for false
			}
			else
				printf("File opened.\n");
		}
		else	//not successful directory change
		{
			printf("directory change failed.\n");
			return 0;
		}
	}
	
	printf("\n");
	printf("Contents from \"%s\" copied to \"%s\".\n", sourceFileName, destinationFileName);	//print statement
	printf("sourcesize:%i\n", sourceArrSize);
	free(sourceFileName);
	free(destinationFileName);
	free(fileContents)
	free(sourcePath);
	free(destinationPath);
	for(int i = 0; i <= MAXPATHLEN ; i++)
	{
		free(sourceArr[i]);
	}
	free(sourceArr);
	for(int i = 0; i <= MAXPATHLEN ; i++)
	{
		free(destinationArr[i]);
	}
	free(destinationArr);
	fclose(sourceFilePtr);	//close source file
	fclose(destinationFilePtr);	//close destination file
	return 1;	//return 1 for a successful copy
}
