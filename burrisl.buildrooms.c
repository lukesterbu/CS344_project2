#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

// Keeps track of room information
struct Room
{
	int number;
	char* name;
	char* type;
	int connections[6];
	int conCount;
};

// Function prototypes
void printRoom(struct Room r[], char* path);
void assignTypes(struct Room r[]);
int connectionExists(struct Room r[], int a, int b);
int graphFull(struct Room r[]);
void connectRoom(struct Room r[], int a, int b);
void makeConnections(struct Room r[]);
void createRoom(struct Room r[], int i, char* name);

int main()
{
	// Use current time as seed for random generator
	srand(time(0));
	char* dirBeg = "burrisl.rooms.";

	// Gets current process ID
	int pid = getpid();

	// Format pid as a string
	int length = snprintf(NULL, 0, "%d", pid);
	char* spid = malloc(length + 1);
	char* dirPath = malloc(length + 2 + strlen(dirBeg));
	strcat(dirPath, dirBeg);
	snprintf(spid, length + 1, "%d", pid);
	strcat(dirPath, spid);
	
	// Make the new directory
	int createDir = mkdir(dirPath, 0755);

	// Make the rooms using color names and put them
	// into a struct room array
	struct Room all[10];
	createRoom(all, 0, "red");
	createRoom(all, 1, "blue");
	createRoom(all, 2, "yellow");
	createRoom(all, 3, "purple");
	createRoom(all, 4, "black");
	createRoom(all, 5, "white");
	createRoom(all, 6, "orange");
	createRoom(all, 7, "green");
	createRoom(all, 8, "pink");
	createRoom(all, 9, "grey");	

	// Fill out room info
	assignTypes(all);
	makeConnections(all);
	// Writes information to file
	printRoom(all, dirPath);

	// Prevent memory leaks
	free(spid);
	free(dirPath);
	spid = NULL;
	dirPath = NULL;

	return 0;
}

// Writes room information to files for use in .adventure executable
void printRoom(struct Room r[], char* path)
{
	int roomNum = 0;
	int cons = 0, length = 0;
	char* consChar;
	int pathLength = strlen(path);
	char* saveName;
	char* roomName;
	char* connection;
	char* roomType;
	int fileDescriptor;
	// 10 possible rooms
	while (roomNum < 10)
	{
		if (r[roomNum].type != NULL)
		{
			// Allocate memory for file name
			saveName = malloc(pathLength + 1 + strlen(r[roomNum].name));
			// Build up save path for new file
			strcat(saveName, path);
			strcat(saveName, "/");
			strcat(saveName, r[roomNum].name);
			// Create file
			fileDescriptor = open(saveName, O_WRONLY | O_CREAT, 0644);
			if (fileDescriptor < 0)
			{
				fprintf(stderr, "Could not open %s\n", saveName);
				perror("Error in printRoom()");
				exit(1);
			}
			// ******************** ROOM NAME ******************
			roomName = malloc(strlen("ROOM NAME: ") + strlen(r[roomNum].name + 1));
			strcat(roomName, "ROOM NAME: ");
			strcat(roomName, r[roomNum].name);
			strcat(roomName, "\n");
			// Write information to file	
			write(fileDescriptor, roomName, strlen(roomName) * sizeof(char));
			// ******************* CONNECTIONS ******************
			// Print connections
			cons = 0;
			length = 0;
			while (cons < r[roomNum].conCount)
			{
				// Convert connection number to string
				length = snprintf(NULL, 0, "%d", cons + 1);
				consChar = malloc(length + 3);	
				snprintf(consChar, length + 1, "%d", cons + 1);
				strcat(consChar, ": ");
				// Build up connection string by allocating memory
				connection = malloc(strlen("CONNECTION ") + strlen(consChar) 
					+ strlen(r[r[roomNum].connections[cons]].name) + 1);
				strcat(connection, "CONNECTION ");
				strcat(connection, consChar);
				strcat(connection, r[r[roomNum].connections[cons]].name);
				strcat(connection, "\n"); 
				// Write string to file
				write(fileDescriptor, connection, strlen(connection) * sizeof(char));
				cons++;
			}
			// ******************** ROOM TYPE ***********************
			// Allocate memory for building room type string
			roomType = malloc(strlen("ROOM TYPE: ") + strlen(r[roomNum].type) + 1);
			strcat(roomType, "ROOM TYPE: ");
			strcat(roomType, r[roomNum].type);
			strcat(roomType, "\n");
			write(fileDescriptor, roomType, strlen(roomType) * sizeof(char));
			// Close file
			close(fileDescriptor);
		}
		roomNum++;
	}
	// Prevent memory leaks
	free(saveName);
	free(roomName);
	free(connection);
	free(roomType);
	// Set all pointers to null
	saveName = NULL;
	roomName = NULL;
	connection = NULL;
	roomType = NULL;
}

// Assigns START_ROOM, MID_ROOM, or END_ROOM to each
// of the rooms being used in this run
void assignTypes(struct Room r[])
{
	int count = 0, startCount = 0, endCount = 0;
	int roomNum = 0, roomType = 0;
	while (count < 7)
	{
		roomNum = rand() % 10;
		roomType = rand() % 3 + 1;
		if (r[roomNum].type == NULL)
		{
			if (roomType == 1 && startCount == 0) 
			{
				r[roomNum].type = "START_ROOM";
				startCount++;
				count++;
			}
			else if (roomType == 2 && (startCount == 1 && endCount == 1))
			{
				r[roomNum].type = "MID_ROOM";
				count++;
			}
			else if (roomType == 3 && endCount == 0)
			{
				r[roomNum].type = "END_ROOM";
				endCount++;
				count++;
			}
		}
	}
}

// Checks if rooms are already connected
// Returns 1 if true, 0 if false
int connectionExists(struct Room r[], int a, int b)
{
	int i = 0;
	while (i < r[a].conCount && i < r[b].conCount)
	{
		if (r[a].connections[i] == b)
			return 1;
		if (r[b].connections[i] == a)
			return 1;
		i++;	
	}
	return 0;
}

// Checks if all rooms have 3 to 6 outbound connections
// Returns 1 if true, 0 if false
int graphFull(struct Room r[])
{
	int i = 0;
	while (i < 10)
	{
		if (r[i].type != NULL && r[i].conCount < 3)
			return 0; 
		i++;
	}
	return 1;
}

// Connects rooms a and b together
void connectRoom(struct Room r[], int a, int b)
{
	r[a].connections[r[a].conCount] = b;
	r[b].connections[r[b].conCount] = a;
	r[a].conCount++;
	r[b].conCount++;
}

// Creates connections between rooms
// code adapted from 2.2 program outlining
void makeConnections(struct Room r[])
{
	int a;
	int b;
	// While the graph does not meet the specifications
	while (!graphFull(r))
	{
		while(1)
		{
			a = rand() % 10;
			if (r[a].type != NULL && r[a].conCount < 6)
				break;
		}	
		do
		{
			b = rand() % 10;
		}
		while (r[b].type == NULL || r[b].conCount > 5 || a == b || connectionExists(r, a, b));
		connectRoom(r, a, b);
	}
}

// Kind of like a contstructor for rooms struct
void createRoom(struct Room r[], int i, char* name)
{
	r[i].number = i + 1;
	r[i].name = name;
	r[i].type = NULL;
	// Set these all to -1 for checking connections later
	r[i].connections[0] = -1;	
	r[i].connections[1] = -1;	
	r[i].connections[2] = -1;	
	r[i].connections[3] = -1;	
	r[i].connections[4] = -1;	
	r[i].connections[5] = -1;
	// Set connection counts to 0
	r[i].conCount = 0;	
}
