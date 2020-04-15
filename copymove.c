#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#define MAXPATHLEN 5	//subdirectory max
#define PATHMAX 256		//space for path
#define FILENAMESIZE 256	//file name size
#define BUFFSIZE 256	//buffer space
// UNDONE: function only copies in same directory
// TODO: copy to different location(aka move)

int copy(char*, char*);

int main(int argc, char* argv[])
{
	char* sourceFile = argv[1],
		* destinationFile = argv[2];
	int result;
	result = copy(sourceFile, destinationFile);
	if(result == 1)
	{
		printf("Copy Successful.\n");
	}
	//printf("back in main\n");
}


/*	Function copies contents of a file to a specified destination
*	arguments: sourceFile(file to be copied) and destinationFile(file contents are copied to
*	returns 0 if failed, 1 if successful
*/
int copy(char* sourceFile, char* destination)
{
	FILE* sourceFilePtr,		//pointer for sourcefile
		* destinationFilePtr;	//pointer for destination file
	char currWD[PATH_MAX],		//current working directory
		* token,	//token for extracting strings
		* sourceFileName,	//holds filename
		* destinationFileName, //holds destination name
		* sourcePath,				//holds source path
		* destinationPath,		//holds destination path
		* fileContents,		//holds contents of file
		** sourceArr,			//array to hold path for source file
		** destinationArr;	//array to hold path for destination file
	int letter,	//data to be copied
		pathLevel = 0,	//count for pathing purpose
		sourcePathSize	= 0,	//size of source pathing
		destinationPathSize = 0,	//size of destination pathing
		sourceArrSize = 0,	//size of source array
		destinationArrSize = 0, //size of destination array
		directoryChanged = 0,	//return value for chdir function
		contentCounter = 0,	//size of source file
		buffCounter = 0;	//buffer counter

	//source file
	sourceArr = malloc((MAXPATHLEN) * sizeof(char)); //allocate space for source array
	if (sourceArr == NULL)	//if space could not be allocated
		printf("Could not allocate sourceArr\n");
	//for loop to allocate space per each individual item
	for(int i = 0; i < MAXPATHLEN; i++)
	{
		sourceArr[i] = malloc(FILENAMESIZE * sizeof(char));
		if(sourceArr[i] == NULL)
			printf("Could not allocate space.\n");
	}
	//parse path and file name
	printf("\tSource file\n");
	getcwd(currWD, sizeof(currWD));		//gets current working directory
	printf("CurrWD before:%s\n",currWD);
	printf("path:%s\n",sourceFile);	
	token = strtok(sourceFile, "/\\\n");	//tokenizes source path(accounts if pathing is given or not)
	while (token != NULL)	//loop to grab rest of the string(path)
	{
		//printf("token:%s\tarrsize:%i\n", token,sourceArrSize);
		strcpy(sourceArr[sourceArrSize], token);	//copies token into source array
		token = strtok(NULL, "/\\\n");	//gets next token
		if(token == NULL)	//break out in case of next token being null
			break;	
		else{
			pathLevel++;		//increase path level counter
			sourceArrSize++;	//increase array counter
		}
		if(pathLevel > PATHMAX)	//section to reallocate space for source path variable
		{
			sourceArr = realloc(sourceArr, PATHMAX);	//realloc space
			pathLevel = 0;	//reset path counter
		}
	}
	//printf("pathlevel:%i\n", sourceArrSize);
	sourceFileName = malloc(FILENAMESIZE * sizeof(char));	//allocate space for source file name
	strcpy(sourceFileName, sourceArr[sourceArrSize]);	//sourcearrsize should be the file name, copies into variable sourceFileName

	if (sourceArrSize == 0)	//case for current working directory
	{
		//printf("in current directory\n");
		sourceFilePtr = fopen(sourceFileName, "r"); //opens the source file
		if (sourceFilePtr == NULL)	//if source file couldn't be opened
		{
			printf("File \"%s\" not found.\n", sourceFile);	//print statement
			return 0;		//return 0 for false
		}
	}
	else	//case for a path given
	{
		//printf("in directories\n");
		sourcePath = malloc(PATHMAX * sizeof(char));	//allocate space for source path
		strcpy(sourcePath, sourceArr[0]);	//gets initial path
		//loop to piece the whole path together
		for (int i = 1; i <= sourcePathSize - 1; i++)
		{
			strcat(sourcePath, "/");
			strcat(sourcePath, sourceArr[i]);
		}
		//printf("%s\n",sourcePath);
		directoryChanged = chdir(sourcePath);	//changes to directory where file is located
		if(directoryChanged == 0)	//if successful
		{
		//	printf("directory changed.\n");
			sourceFilePtr = fopen(sourceFileName, "r"); //opens the source file
			if (sourceFilePtr == NULL)	//if source file couldn't be opened
			{
				printf("File \"%s\" not found.\n", sourceFileName);	//print statement
				return 0;		//return 0 for false
			}
			//else
				//printf("File opened.\n");
		}
		else	//not successful directory change
		{
			printf("directory change failed.\n");
			return 0;
		}
	}
	//printf("sourcearrsize:%i\n",sourceArrSize);
	/*for(int i = 0; i <= sourceArrSize; i ++)
	{
		printf("%i:%s\n",i, sourceArr[i]);
	}*/
	getcwd(currWD, sizeof(currWD));	//gets current directory
	printf("CurrWD after:%s\n",currWD);
	free(sourceArr);	//frees source array
	//printf("sourcearr freed\n");

	//copy
	//printf("\n");
	printf("\n\n\tCopying file\n");
	//printf("contents:");
	fileContents = malloc(BUFFSIZE * sizeof(char));	//allocate space for file contents
	while ((letter = fgetc(sourceFilePtr)) != EOF)	//while letter is not at the end of the file
	{
		fileContents[contentCounter] = (char) letter;	//copies letter into filecontents
		//printf("%c", (char)fileContents[contentCounter]);
		contentCounter++;	//increase content counter
		buffCounter++;	//increase buffer counter
		if(buffCounter >= BUFFSIZE )	//checks if array is full
		{
			//printf("\n");
			//printf("need to realloc\n");
			fileContents = realloc(fileContents, BUFFSIZE);	//realloc space
			buffCounter = 0;	//reset buffer counter
			if(fileContents == NULL)	//not able to realloc space
				printf("could not realloc\n");
		}
	}
	fileContents[contentCounter] = '\0';	//null terminator for string
	fclose(sourceFilePtr);	//close source file
	//printf("source closed\n");
	chdir("/");	//change to root
	chdir(currWD);	//change back to previous curr working directory
	getcwd(currWD, sizeof(currWD));	//gets current working directory
	//printf("CurrWD:%s\n",currWD);
	printf("copy complete.\n");

	//destination file
	printf("\n\n\tDestination file\n");
	getcwd(currWD, sizeof(currWD));	//gets current working directory
	printf("CurrWD before:%s\n",currWD);
	destinationArr = malloc((MAXPATHLEN) * sizeof(char));	//allocates space for detination array
	if(destinationArr == NULL)	//space could not be allocated
		printf("could not allocate destinationArr\n");
	//for loop to allocate space per each individual item
	for(int i = 0; i < MAXPATHLEN; i++)
	{
		destinationArr[i] = malloc(FILENAMESIZE * sizeof(char));
		if(destinationArr[i] == NULL)
			printf("Could not allocate space.\n");
	}
	printf("path:%s\n",destination);
	token = strtok(destination, "/\\\n");	//tokenizer for pathing
	pathLevel = 0;	//resets path level
	while (token != NULL)	//loop to grab rest of the string(path)
	{
		//printf("token:%s\t arrsize:%i\n", token,destinationArrSize);
		strcpy(destinationArr[destinationArrSize], token);	//copies token into source array
		token = strtok(NULL, "/\\\n");	//gets next token
		if(token == NULL)	//token is null break out of loop
			break;	
		else{
			pathLevel++;		//increase path level counter
			destinationArrSize++;	//increase array counter
		}
		if(pathLevel > PATHMAX)	//section to reallocate space for source path variable
		{
			destinationArr = realloc(destinationArr, PATHMAX);	//reallocate space
			pathLevel = 0;	//reset path level
		}
	}
	//printf("pathlevel:%i\n",destinationArrSize);
	destinationFileName = malloc(FILENAMESIZE * sizeof(char));	//allocate space for destination file name
	strcpy(destinationFileName, destinationArr[destinationArrSize]);	//pathlevel should be the file name, copies into variable sourceFileName
	/*
	for(int i = 0; i <= destinationArrSize; i++)
	{
		printf("%i:%s\n",i, destinationArr[i]);
	}*/
	if (destinationArrSize == 0)	//case for current working directory
	{
		//printf("in current directory for destination\n");
		destinationFilePtr = fopen(destinationFileName, "w"); //opens the source file
		if (destinationFilePtr == NULL)	//if source file couldn't be opened
		{
			printf("File \"%s\" could not be written to.\n", destinationFileName);	//print statement
			//return 0;		//return 0 for false
		}
		//printf("contents:%s\n",fileContents);
		fprintf(destinationFilePtr, "%s", fileContents);	//copies contents to new area
		//printf("done copying\n");
	}
	else	//case for a path given
	{
		//printf("in directories\n");
		destinationPath = malloc(PATHMAX * sizeof(char));	//allocate space for destination path
		strcpy(destinationPath, destinationArr[0]);	//gets initial path
		//loop to piece the whole path together
		for (int i = 1; i <= destinationArrSize-1; i++)
		{
			strcat(destinationPath, "/");
			strcat(destinationPath, destinationArr[i]);
			//printf("%i:%s\n",i,destinationArr[i]);
		}
		//printf("destinationpath:%s\n",destinationPath);
		directoryChanged = chdir(destinationPath);	//changes to directory where file is located
		if(directoryChanged == 0)	//if successful
		{
			//printf("directory changed.\n");
			destinationFilePtr = fopen(destinationFileName, "w"); //opens the source file
			if (destinationFilePtr == NULL)	//if source file couldn't be opened
			{
				printf("File \"%s\" cannot be written to.\n", destinationFileName);	//print statement
				return 0;		//return 0 for false
			}
			else
			{
				//printf("File opened.\n");
				//printf("contents:%s\n",fileContents);
				fprintf(destinationFilePtr, "%s", fileContents); //copy contents to file
				//printf("done copying\n");
			}
		}
		else	//not successful directory change
		{
			printf("directory change failed.\n");
			return 0;
		}
	}
	getcwd(currWD, sizeof(currWD));
	printf("CurrWD after:%s\n",currWD);
	free(destinationArr);	//frees destination array
	//printf("destinationarr freed\n");
	fclose(destinationFilePtr);	//close destination file
	//printf("destination closed\n");

	printf("\n");
	//printf("Contents from \"%s\" copied to \"%s\".\n", sourceFileName, destinationFileName);	//print statement
	
	//printf("\n\tFree Time\n");
	free(sourceFileName);
	//printf("Sourcefilename freed\n");
	free(destinationFileName);
	//printf("destinationfilename freed\n");
	free(fileContents);
	//printf("filecontents freed\n");
	free(sourcePath);
	//printf("sourcepath freed\n");
	free(destinationPath);
	//printf("destinationpath freed\n");
	return 1;	//return 1 for a successful copy
}
