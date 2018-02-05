#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
//#include<io.h> //for windows

//macros
#define _CRT_SECURE_NO_WARNINGS //to hide warnings on windows
#define MAXINODE 50
#define READ 1
#define WRITE 2
#define MAXFILESIZE 1024
#define REGULAR 1
#define SPECIAL 2
#pragma warning(disable:4996)

//parameters of lseek
#define START 0
#define CURRENT 1
#define END 2


//structures
typedef struct superblock
{
  int TotalInodes;
  int FreeInode;
}SUPERBLOCK, *PSUPERBLOCK;

typedef struct inode
{
  char FileName[50];
  int InodeNumber;
  int FileSize;
  int FileActualSize;
  int FileType;
  char *buffer;
  int LinkCount;
  int ReferenceCount;
  int permission;
  struct inode *next;
}INODE,*PINODE,**PPINODE;

typedef struct filetable
{
  int readoffset;
  int writeoffset;
  int count;
  int mode;
  PINODE ptrinode;
}FILETABLE,*PFILETABLE;

typedef struct ufdt
{
  PFILETABLE ptrfiletable;
}UFDT;

//global variables
UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;

//manual fuction
void man(char *name) //manual
{
  if(name == NULL)
    {
      return;
    }
  if(strcmp(name,"create") == 0)
    {
      printf("Description: Used to create new regular file\n");
      printf("Usage: Create File_name Permission\n");
    }
  else if(strcmp(name,"read") == 0)
    {
      printf("Description: Used to read data from regular file\n");
      printf("Usage: read File_name No_Of_Bytes_To_Read\n");
    }
  else if(strcmp(name,"write") == 0)
    {
      printf("Description: Used to write into regular file\n");
      printf("Usage: write File_name\n After this enter the data that we want to write\n");
    }
  else if(strcmp(name,"ls") == 0)
    {
      printf("Description: Used to list all information of files\n");
      printf("Usage: ls\n");
    }
  else if(strcmp(name,"stat") == 0)
    {
      printf("Description: Used to list allto display information of files\n");
      printf("Usage: stat File_Name\n");
    }
  else if(strcmp(name,"fstat") == 0)
    {
      printf("Description: Used to display information of Files\n");
      printf("Usage: stat File_Name\n");
    }
  else if(strcmp(name,"truncate") == 0)
    {
      printf("Description: Used to remove data from files\n");
      printf("Usage: truncate File_Name\n");
    }
  else if(strcmp(name,"open") == 0)
    {
      printf("Description: Used to open the existing file\n");
      printf("Usage: open File_Name\n");
    }
  else if(strcmp(name,"close") == 0)
    {
      printf("Description: Used to close the opened file\n");
      printf("Usage: close File_Name\n");
    }
  else if(strcmp(name,"closeall") == 0)
    {
      printf("Description: Used to close all files which are open.\n");
      printf("Usage: closeall\n");
    }
  else if(strcmp(name,"lseek") == 0)
    {
      printf("Description: Used to change the offset\n");
      printf("Usage: lseek File_Name ChangeInOffset StartPoint\n");
    }
  else if(strcmp(name,"rm") == 0)
    {
      printf("Description: Used to delete the file\n");
      printf("Usage: rm File_Name\n");
    }
  else
    {
      printf("ERROR: No manual entry available \n");
    }
}


//function which displays help to the end user
void DisplayHelp()
{
  printf("ls: To list out all files\n");
  printf("clear: To clear console\n");
  printf("open: To open the file\n");
  printf("close: To close the file\n");
  printf("closeall: To close all the files\n");
  printf("read: To read the contents from the file\n");
  printf("write: To write the contents into the file\n");
  printf("exit: To terminate file system\n");
  printf("stat: To display information of file by using name\n");
  printf("fstat: To display information of file by using file descriptor\n");
  printf("truncate: To Remove all data from file\n");
  printf("rm: To delete the files\n");
}

//function to get file descriptor
int GetFDFromName(char *name)
{
	int i = 0;
	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			if (strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
			{
				break;
			}
			i++;
		}
		if (i == 50)
		{
			return -1;
		}
		else
		{
			return i;
		}
	}
}


PINODE Get_Inode(char *name)
{
	PINODE temp = head;
	if (name == NULL)
	{
		return NULL;
	}
	while (temp != NULL) //traverse 50 times
	{
		if (strcmp(name, temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	return temp;
}

     

//function to initialization of super block
void InitializeSuperBlock()
{
	int i = 0;
	while (i < MAXINODE)
	{
		UFDTArr[i].ptrfiletable = NULL;
		i++;
	}
	SUPERBLOCKobj.TotalInodes = MAXINODE;
	SUPERBLOCKobj.FreeInode = MAXINODE;
}

//function to create Data Inode List Block
void CreateDILB()
{
	PINODE newn = NULL;
	PINODE temp = head;
	int i = 1;
	while (i <= MAXINODE)
	{
		printf("hello\n");
		newn = (PINODE)malloc(sizeof(INODE));
		printf("chetan \n\n\n");
		newn->LinkCount = newn->ReferenceCount = 0;
printf("chetanaaa \n");
		newn->FileType = newn->FileSize = 0;
printf("chetan \n");
		newn->buffer = NULL;
		newn->next = NULL;
		newn->InodeNumber = i;
printf("sdssdsd \n");
		if (temp == NULL)
		{
			head = newn;
			temp = head;
		}
		else
		{
			temp -> next = newn;
			temp = temp->next;
		}
		i++;
printf("chetan \n");
	}
}

int CreateFile(char *name, int permission)
{
	int i = 0;
	PINODE temp = head;

	if ((name == NULL) || (permission == 0) || (permission > 3))
	{
		return -1;
	}

	if (SUPERBLOCKobj.FreeInode == 0)
	{
		return -2;
	}
	if (Get_Inode(name) != NULL)
	{
		return -3;
	}

	(SUPERBLOCKobj.FreeInode)--;

	while (temp != NULL)
	{
		if (temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}

	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -4;
	}

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	UFDTArr[i].ptrfiletable->ptrinode = temp;
	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
	UFDTArr[i].ptrfiletable->ptrinode->buffer = (char *)malloc(MAXFILESIZE); //1024 bytes allocated
	return i;
}

int rm_File(char *name)
{
	int fd = 0;
	fd = GetFDFromName(name);
	if (fd == -1)
	{
		return -1;
	}

	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

	if (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
		free(UFDTArr[fd].ptrfiletable);
	}
	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInode)++;
}

int ReadFile(int fd, char *arr, int isize)
{
	int read_size = 0;
	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}

	if (UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -2;
	}

	if ((UFDTArr[fd].ptrfiletable->ptrinode->permission != READ) && (UFDTArr[fd].ptrfiletable->ptrinode->permission != READ + WRITE))
	{
		return -3;
	}

	if (UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -4;
	}

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

	if (read_size < isize)
	{
		strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->buffer) + (UFDTArr[fd].ptrfiletable->readoffset), read_size);

		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
	}
	else
	{
		strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->buffer) + (UFDTArr[fd].ptrfiletable->readoffset), isize);

		UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + isize;
	}
	return isize;
}

int WriteFile(int fd, char *arr, int isize)
{
	if (((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))
	{
		return -1;
	}

	if (((UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ + WRITE))
	{
		return -1;
	}

	if ((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
	{
		return -2;
	}

	if ((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
	{
		return -3;
	}

	strncpy((UFDTArr[fd].ptrfiletable->ptrinode->buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, isize);

	(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;

	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

	return isize;
}

int OpenFile(char *name, int mode)
{
	int i = 0;
	PINODE temp = NULL;
	if (name == NULL || mode <= 0)
	{
		return -1;
	}

	temp = Get_Inode(name);
	if (temp == NULL)
	{
		return -2;
	}

	if (temp->permission < mode)
	{
		return -3;
	}

	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -1;
	}

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = mode;

	if (mode == READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	else if (mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
	}
	else if (mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	UFDTArr[i].ptrfiletable->ptrinode = temp;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

	return i;
}

void CloseFileByFD(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

int CloseFileByName(char *name)
{
	int i = 0;

	i = GetFDFromName(name);
	if (i == -1)
	{
		return -1;
	}
			
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

		return 0;
}

void closeAllFile()
{
	int i = 0;
	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;
			UFDTArr[i].ptrfiletable->writeoffset = 0;
			(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
		}
		i++;
	}
}

//lseek function

int LseekFile(int fd, int size, int from)
{
	if ((fd<0) || (from >2))
		return -1;
	if (UFDTArr[fd].ptrfiletable == NULL)
		return -1;

	if ((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
	{
		if (from == CURRENT)
		{
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
			{
				return -1;
			}

			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
		}
		else if (from == START)
		{
			if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				return -1;
			}
			if (size < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = size;
		}

		else if (from == END)
		{
			if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
			{
				return -1;
			}

			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
		}
	}
	else if (UFDTArr[fd].ptrfiletable->mode == WRITE)
	{
		if (from == CURRENT)
		{
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
			}
			(UFDTArr[fd].ptrfiletable->writeoffset = (UFDTArr[fd].ptrfiletable->writeoffset) + size);
		}
		else if (from == START)
		{
			if (size > MAXFILESIZE)
			{
				return -1;
			}
			if (size < 0)
			{
				return -1;
			}
			if (size >(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
			}

			(UFDTArr[fd].ptrfiletable->writeoffset) = size;
		}
		else if (from == END)
		{
			if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
		}
	}
}

void ls_file()
{
	PINODE temp = head;

	if (SUPERBLOCKobj.FreeInode == MAXINODE)
	{
		printf("ERROR: There are no files\n");
		return;
	}

	printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
	printf("-----------------------------------------------------\n");

	while (temp != NULL)
	{
		if (temp->FileType != 0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
		}
		temp=temp->next;
	}
	printf("---------------------------------------------------\n");
}

//this function displays information by file descriptor
int fstat_file(int fd)
{
	PINODE temp = head;

	if (fd < 0)
	{
		return -1;
	}

	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		return -2;
	}

	temp = UFDTArr[fd].ptrfiletable->ptrinode;

	printf("\n--------Statistical Information about file-------\n");
	printf("File Name: %s\n", temp->FileName);
	printf("Inode Number: %d\n", temp->InodeNumber);
	printf("File size: %d\n", temp->FileSize);
	printf("Actual File Size: %d\n", temp->FileActualSize);
	printf("Link Count: %d\n", temp->LinkCount);
	printf("Reference Count: %d\n", temp->ReferenceCount);
	if (temp->permission == 1)
	{
		printf("File Permission: Read only\n");
	}
	else if (temp->permission == 2)
	{
		printf("File Permission: write\n");
	}
	else if (temp->permission == 3)
	{
		printf("File Permission: Read & write\n");
	}
	printf("----------------------------------------------------\n");
	return 0;
}

//this function displays information by name
int stat_file(char *name)
{
	PINODE temp = head;

	if (name == NULL)
	{
		return -1;
	}

	while (temp != NULL)
	{
		if (strcmp(name, temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}

	if (temp == NULL)
	{
		return -2;
	}

	printf("\n--------Statistical Information about file-------\n");
	printf("File Name: %s\n", temp->FileName);
	printf("Inode Number: %d\n", temp->InodeNumber);
	printf("File size: %d\n", temp->FileSize);
	printf("Actual File Size: %d\n", temp->FileActualSize);
	printf("Link Count: %d\n", temp->LinkCount);
	printf("Reference Count: %d\n", temp->ReferenceCount);
	if (temp->permission == 1)
	{
		printf("File Permission: Read only\n");
	}
	else if (temp->permission == 2)
	{
		printf("File Permission: write\n");
	}
	else if (temp->permission == 3)
	{
		printf("File Permission: Read & write\n");
	}
	printf("----------------------------------------------------\n");
	return 0;
}

//truncate
int truncate_File(char *name)
{
	int fd = GetFDFromName(name);
	if (fd == -1)
	{
		return -1;
	}

	memset(UFDTArr[fd].ptrfiletable->ptrinode->buffer, 0, 1024);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

//entry point function
int main()
{
	char *ptr = NULL;
	int ret = 0, fd = 0, count = 0;
	char command[4][8], str[80], arr[1024];
	printf("chetan \n\n\n");
	InitializeSuperBlock();
	CreateDILB();
	while (1)
	{
		fflush(stdin);
		strcpy(str, "");

		printf("\nMy VFS: >");
		fgets(str, 80, stdin);

		count = sscanf(str, "%s%s%s%s", command[0], command[1], command[2], command[3]); //tokenized

		if (count == 1)
		{
			if (strcmp(command[0], "ls") == 0)
			{
				ls_file();
			}
			else if (strcmp(command[0], "closeall") == 0)
			{
				closeAllFile();
				printf("All files closed successfully\n");
				continue;
			}
			else if (strcmp(command[0], "clear") == 0)
			{
				system("clear");
				continue;
			}
			else if (strcmp(command[0], "help") == 0)
			{
				DisplayHelp();
				continue;
			}
			else if (strcmp(command[0], "exit") == 0)
			{
				printf("Terminating My Virtual File System\n");
				break;
			}

			else
			{
				printf("ERROR: Command not found\n");
				continue;
			}
		}
		else if (count == 2)
		{
			if (strcmp(command[0], "stat") == 0)
			{
				ret = stat_file(command[1]);
				if (ret == -1)
				{
					printf("ERROR: Incorrect Parameters\n");
				}
				if (ret == -2)
				{
					printf("ERROR: There is no such file\n");
				}
				continue;
			}
			else if (strcmp(command[0], "fstat") == 0)
			{
				ret = fstat_file(atoi(command[1]));
				if (ret == -1)
				{
					printf("ERROR: Incorrect parameters\n");
				}
				if (ret == -2)
				{
					printf("ERROR: There is no such file\n");
				}
				continue;
			}
			else if (strcmp(command[0], "close") == 0)
			{
				ret = CloseFileByName(command[1]);

				if (ret == -1)
				{
					printf("ERROR: There is no such file\n");
				}
				continue;
			}

			else if (strcmp(command[0], "rm") == 0)
			{
				ret = rm_File(command[1]);
				if (ret == -1)
				{
					printf("ERROR: There is no such file\n");
				}
				continue;
			}

			else if (strcmp(command[0], "man") == 0)
			{
				man(command[1]);
			}

			else if (strcmp(command[0], "write") == 0)
			{
				fd = GetFDFromName(command[1]);
				if (fd == -1)
				{
					printf("ERROR: Incorrect parameters\n");
					continue;
				}
				printf("Enter the data: \n");
				scanf("%[^\n]", arr);

				ret = strlen(arr);
				if (ret == 0)
				{
					printf("ERROR: Incorrect parameters\n");
					continue;
				}
				ret = WriteFile(fd, arr, ret);
				if (ret == -1)
				{
					printf("ERROR: Permission denied\n");
				}
				if (ret == -2)
				{
					printf("ERROR: There is no sufficient memory to write\n");
				}
				if (ret == -3)
				{
					printf("ERROR: It is not regular file\n");
				}
			}
			else if (strcmp(command[0], "truncate") == 0)
			{
				ret = truncate_File(command[1]);
				if (ret == -1)
				{
					printf("ERROR: Incorrect parameters\n");
				}
			}
			else
			{
				printf("\n ERROR: Command not found !!\n");
				continue;
			}
		}
		else if (count == 3)
		{
			if (strcmp(command[0], "create") == 0)
			{
				
				ret = CreateFile(command[1], atoi(command[2]));
				if (ret >= 0)
				{
					printf("File is Successfully created with file descriptor : %d\n", ret);
				}
				if (ret == -1)
				{
					printf("ERROR: Incorrect parameters\n");
				}
				if (ret == -2)
				{
					printf("ERROR: There is no inodes\n");
				}
				if (ret == -3)
				{
					printf("ERROR: File is already exist\n");
				}
				if (ret == -4)
				{
					printf("ERROR: Memory allocation failure\n");
				}
				continue;
			}
			else if (strcmp(command[0], "open") == 0)
			{
				ret = OpenFile(command[1], atoi(command[2]));

				if (ret >= 0)
				{
					printf("File is successfully opened with file descriptor: %d\n", ret);
				}
				if (ret == -1)
				{
					printf("ERROR: Incorrect parameters\n");
				}
				if (ret == -2)
				{
					printf("ERROR: File is not present\n");
				}
				if (ret == -3)
				{
					printf("ERROR: Permission denied\n");
				}
				continue;
			}
			else if (strcmp(command[0], "read") == 0)
			{
				fd = GetFDFromName(command[1]);
				if (fd == -1)
				{
					printf("ERROR: Incorrect Parameters\n");
					continue;
				}
				ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);
				if (ptr == NULL)
				{
					printf("ERROR: Memory allocation failure\n");
					continue;
				}

				ret = ReadFile(fd, ptr, atoi(command[2]));
				if (ret == -1)
				{
					printf("ERROR: File not existing\n");
				}
				if (ret == -2)
				{
					printf("ERROR: Permission denied\n");
				}
				if (ret == -3)
				{
					printf("ERROR: Reached at the end of the line\n");
				}
				if (ret == -4)
				{
					printf("ERROR: It is not regular file\n");
				}
				if (ret == 0)
				{
					printf("ERROR: File is Empty\n");
				}
				if (ret > 0)
				{
					write(2, ptr, ret);
				}
				continue;
			}
			else
			{
				printf("\nERROR: Command not found !!!\n");
				continue;
			}
		}
		else if (count == 4)
		{
			if (strcmp(command[0], "lseek") == 0)
			{
				fd = GetFDFromName(command[1]);
				if (fd == -1)
				{
					printf("ERROR: Incorrect parameters\n");
					continue;
				}
				ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
				if (ret == -1)
				{
					printf("ERROR: Unable to perform lseek\n");
				}
			}
			else
			{
				printf("ERROR: Command not found\n");
				continue;
			}
		}
		else
		{
			printf("ERROR: Command not found\n");
			continue;
		}
	}
	return 0;
}
