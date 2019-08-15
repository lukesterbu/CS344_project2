#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>

// Global mutex
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

// Keeps track of room information
struct Room
{
	char name[100];
	char type[100];
	char connections[6][30];
	int conCount;
};

// Function prototypes
struct Room* getRoom(struct Room r[], char* name);
struct Room* checkConnection(struct Room r[], struct Room* currRoom, char* room);
struct Room* prompt(struct Room r[], struct Room* currRoom);
void printCurrentTime();
void* writeTimeToFile();
time_t getModifiedTime(char* dirPath);
char* latestDir();
void substring(char s[], char sub[], int p, int l);
void loadRooms(char* roomDir, struct Room r[]);
char* getFormattedConnections(struct Room* r);

int main() 
{
	struct Room rooms[7];
	struct Room* currRoom;
	int i = 0, steps = 0;
	char roomsVisited[20][20];
	// Set all room connection counts to 0
	while (i < 7)
	{
		rooms[i].conCount = 0;
		i++;
	}
	// Loads room information from files into struct array
	loadRooms(latestDir(), rooms);
	i = 0;
	while(i < 7)
	{
		if (strcmp(rooms[i].type, "START_ROOM") == 0)
			currRoom = &rooms[i];
		i++; 
	}
	// While current room type isn't END_ROOM
	while(strcmp(currRoom->type, "END_ROOM") != 0)
	{
		currRoom = prompt(rooms, currRoom);
		strcpy(roomsVisited[steps], currRoom->name);
		steps++;
	}
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	i = 0;
	while(i < steps)
	{
		printf("%s\n", roomsVisited[i]);
		i++;
	}
	return 0;
}

// Returns the room associated with the passed name otherwise null
struct Room* getRoom(struct Room r[], char* name)
{
	int i = 0;
	while(i < 7)
	{
		if (strcmp(r[i].name, name) == 0)
		{
			return &r[i];
		}
		i++;
	}
	return NULL;
}

// Returns room if connection exists otherwise null
struct Room* checkConnection(struct Room r[], struct Room* currRoom, char* room)
{
	int i = 0;
	while(i < currRoom->conCount)
	{
		if (strcmp(currRoom->connections[i], room) == 0)
		{
			return getRoom(r, room);
		}
		i++;
	}
	return NULL;
}

// Prints prompt and gets input from user
struct Room* prompt(struct Room r[], struct Room* currRoom)
{
	size_t bufSize = 100;
	char* buf = malloc(bufSize * sizeof(char));
	struct Room* check;
	printf("CURRENT LOCATION: %s\n", currRoom->name);
	printf("POSSIBLE CONNECTIONS: %s\n", getFormattedConnections(currRoom));
	do
	{
		printf("WHERE TO? >");
		getline(&buf, &bufSize, stdin);
		printf("\n");
		strtok(buf, "\n");
		// Check if user entered time
		if (strcmp(buf, "time") == 0)
		{
			printCurrentTime();
			continue; // skip to next iteration
		}
		check = checkConnection(r, currRoom, buf);
		// Check if user entered room that doesn't exist
		if (check == NULL)
		{
			printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
			printf("CURRENT LOCATION: %s\n", currRoom->name);
			printf("POSSIBLE CONNECTIONS: %s\n", getFormattedConnections(currRoom));
		}
	} while(strcmp(buf, "time") == 0 || check == NULL);
	// Prevent memory leaks
	free(buf);
	buf = NULL;
	return check;
}

// Creates a thread to write the time to a file and then
// opens that file, reads the info, and prints it to the console
void printCurrentTime()
{	
	int result;
	char line[200];
	FILE* file;
	pthread_t myThreadID;
	
	pthread_mutex_lock(&myMutex);
	result = pthread_create(&myThreadID, NULL, writeTimeToFile, NULL);
	
	pthread_mutex_lock(&myMutex);
	if ((file = fopen("currentTime.txt", "r")) == NULL)
	{
		printf("File currentTime.txt does not exist!\n");
		exit(1);
	}
	while (fgets(line, sizeof(line), file) != NULL)
	{
		printf("%s\n", line);	
	}
	pthread_mutex_unlock(&myMutex);
}

// Writes the current time to currentTime.txt
void* writeTimeToFile()
{
	int fileDescriptor = open("currentTime.txt", O_WRONLY | O_CREAT, 0644);
	time_t rawtime;
	struct tm* timeStruct;
	char myTime[100];

	time(&rawtime);
	timeStruct = localtime(&rawtime);
	strftime(myTime, sizeof(myTime), " %l:%M%p, %A, %B %d, %Y\n", timeStruct);
	write(fileDescriptor, myTime, strlen(myTime) * sizeof(char));
	pthread_mutex_unlock(&myMutex);
}

// Returns the latest modified time
time_t getModifiedTime(char* dirPath)
{
	struct stat buf;
	if (stat(dirPath, &buf) == 0)
	{
		return buf.st_mtime;
	}
	return 0;
}

// Returns the latest rooms directory 
char* latestDir()
{
	struct dirent* dirEntry;
	DIR* dir = opendir(".");
	char latestModDir[100];
	char* ptr;
	int count = 0;
	if (dir == NULL)
	{
		printf("No directories exist yet!");
		return;
	}
	while ((dirEntry = readdir(dir)) != NULL)
	{
		if (strstr(dirEntry->d_name, "burrisl.rooms.") != NULL)
		{
			if (count = 0)
			{
				memset(latestModDir, '\0', sizeof(latestModDir));
				strcpy(latestModDir, dirEntry->d_name);
			}
			else
			{	
				time_t t1 = getModifiedTime(latestModDir); 
				time_t t2 = getModifiedTime(dirEntry->d_name);
				if (t1 < t2)
				{
					memset(latestModDir, '\0', sizeof(latestModDir));
					strcpy(latestModDir, dirEntry->d_name);
				}
			}
			count++;
		}
	}
	closedir(dir);
	ptr = latestModDir;
	return ptr;
}

// sub now contains substring
void substring(char s[], char sub[], int p, int l)
{
	int c = 0;
	while (c < l)
	{
		sub[c] = s[p + c - 1];
		c++;
	}
	sub[c] = '\0';
}

// Returns starting room location
void loadRooms(char* roomDir, struct Room r[])
{
	struct dirent* dirEntry;
	DIR* dir = opendir(roomDir);
	FILE* file;
	char line[1000];
	char filePath[100];
	char roomDirCopy[100];
	strcpy(roomDirCopy, roomDir);
	strcat(roomDirCopy, "\0");
	int roomCount = 0;
	if (dir == NULL)
	{
		printf("Directory %s does not exist!\n", roomDir);
		return;
	}
	while ((dirEntry = readdir(dir)) != NULL)
	{
		if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
		{
			// Clear out arrays
			memset(filePath, '\0', sizeof(line));
			memset(line, '\0', sizeof(filePath));
			// Build up filepath
			strcat(filePath, roomDirCopy);
			strcat(filePath, "/");
			strcat(filePath, dirEntry->d_name);
			strcat(filePath, "\0");
			if ((file = fopen(filePath, "r")) == NULL)
			{
				printf("File %s does not exist!\n", filePath);
				exit(1);
			}
			// Read file and put the information into the struct array
			while (fgets(line, sizeof(line), file) != NULL)
			{
				if (strstr(line, "ROOM NAME:") != NULL)
				{
					substring(line, r[roomCount].name, 12, strlen(line));
					strtok(r[roomCount].name, "\n");
				}
				else if (strstr(line, "CONNECTION") != NULL)
				{
					substring(line, r[roomCount].connections[r[roomCount].conCount], 15, strlen(line));
					strtok(r[roomCount].connections[r[roomCount].conCount], "\n");
					r[roomCount].conCount++;
				}
				else if (strstr(line, "ROOM TYPE:") != NULL)
				{
					substring(line, r[roomCount].type, 12, strlen(line));
					strtok(r[roomCount].type, "\n");
				}
			}
			fclose(file);
			roomCount++;
		}
	}
	closedir(dir);
}

// Returns possible connections formatted based on Program 2's specifications
char* getFormattedConnections(struct Room* r)
{
	char* ptr;
	char connections[100];
	int con = 0;
	memset(connections, '\0', sizeof(connections));
	con = 0;
	while (con < r->conCount)
	{
		strcat(connections, r->connections[con]);
		if (con + 1 == r->conCount)
			strcat(connections, ".");
		else
			strcat(connections, ", ");
		con++;
	}
	strcat(connections, "\0");
	ptr = connections;
	return ptr;
}
