#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <unistd.h>
// client

#define MUTEX_CONNECT_NAME "connect"
#define MEM_CONNECT_NAME "connectMem"


int getId(int *id, unsigned int *serverId) {
    HANDLE connectMutex;
    HANDLE connectMapFile;
    LPCTSTR connectPBuf;
    connectMutex = OpenMutexA(
            MUTEX_ALL_ACCESS,
            FALSE,
            MUTEX_CONNECT_NAME);
    if (connectMutex == NULL) {
        return 3;
    }

    connectMapFile = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,   // read/write access
            FALSE,                 // do not inherit the name
            MEM_CONNECT_NAME);               // name of mapping object

    if (connectMapFile == NULL) {
        CloseHandle(connectMutex);
        return 3;
    }

    connectPBuf = (LPTSTR) MapViewOfFile(connectMapFile, // handle to map object
                                         FILE_MAP_ALL_ACCESS,  // read/write permission
                                         0,
                                         0,
                                         6);

    if (connectPBuf == NULL) {
        CloseHandle(connectMutex);
        CloseHandle(connectMapFile);
        return 3;
    }

    DWORD dwWaitResult;
    dwWaitResult = WaitForSingleObject(
            connectMutex,    // handle to mutex
            60000);  // 1 min timeout

    int status = 1;
    int timeout = 0;
    switch (dwWaitResult) {
        case WAIT_OBJECT_0:
            while (status != 0) {
                memcpy(&status, (void *) connectPBuf, sizeof(char));
                sleep(1);
                timeout++;
                if (timeout == 60) {
                    UnmapViewOfFile(connectPBuf);
                    CloseHandle(connectMapFile);
                    CloseHandle(connectMutex);
                    return 7;
                }
            }
            memcpy(id, (void *) (connectPBuf + 1), sizeof(char));
            memcpy(serverId, (void *) (connectPBuf + 2), sizeof(int));
            status = 1;
            memcpy((void *) connectPBuf, &status, sizeof(char));
            break;
        case WAIT_ABANDONED:
            UnmapViewOfFile(connectPBuf);
            CloseHandle(connectMapFile);
            CloseHandle(connectMutex);
            return 4;
        default:
            UnmapViewOfFile(connectPBuf);
            CloseHandle(connectMapFile);
            CloseHandle(connectMutex);
            return 5;
    }
    UnmapViewOfFile(connectPBuf);
    CloseHandle(connectMapFile);
    ReleaseMutex(connectMutex);
    return 0;
}

int getResultId(int *result, unsigned int timeout, int id) {
    HANDLE connectMapFile = NULL;
    LPCTSTR connectPBuf;
    char name[15];
    sprintf(name, "client%d", id);

    while (timeout && connectMapFile == NULL) {
        connectMapFile = OpenFileMapping(
                FILE_MAP_ALL_ACCESS,   // read/write access
                FALSE,                 // do not inherit the name
                name);               // name of mapping object

        sleep(1);
        timeout--;
    }
    if (connectMapFile == NULL) {
        return 3;
    }
    connectPBuf = (LPTSTR) MapViewOfFile(connectMapFile, // handle to map object
                                         FILE_MAP_ALL_ACCESS,  // read/write permission
                                         0,
                                         0,
                                         1);

    if (connectPBuf == NULL) {
        CloseHandle(connectMapFile);
        return 4;
    }

    int data = 0;
    while (timeout != 0) {
        memcpy(&data, (void *) connectPBuf, sizeof(char));
        if (data != 0) {
            *result = data;
            UnmapViewOfFile(connectPBuf);
            CloseHandle(connectMapFile);
            return 0;
        }
        sleep(1);
        timeout--;
    }

    UnmapViewOfFile(connectPBuf);
    CloseHandle(connectMapFile);
    return 2;
}

int readString(FILE *file, char **result, const char *separators, char *metSeparator) {
    int isEOF = 0;
    int resCapacity = 1;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    char c;
    int count = 0;
    while (1) {
        c = fgetc(file);
        if (c == EOF) {
            isEOF = 1;
            break;
        }
        int x = 0;
        int isBreak = 0;
        while (separators[x] != '\0') {
            if (c == separators[x]) {
                if (metSeparator != NULL)
                    *metSeparator = c;
                isBreak = 1;
                break;
            }
            x++;
        }
        if (isBreak)
            break;
        if (count == resCapacity) {
            resCapacity *= 2;
            char *newRes = (char *) realloc(res, sizeof(char) * resCapacity);
            if (newRes == NULL) {
                free(res);
                return 1;
            }
            res = newRes;
        }
        res[count] = c;
        count++;
    }
    if (count == 0) {
        free(res);
        return 3;
    }
    char *newRes = (char *) realloc(res, sizeof(char) * (count + 1));
    if (newRes == NULL) {
        free(res);
        return 1;
    }
    res = newRes;
    res[count] = '\0';
    *result = res;
    if (isEOF)
        return -1;
    return 0;
}

int nextLine(FILE *file) {
    char c;
    int isEOF = 0;
    while (1) {
        c = fgetc(file);
        if (c == EOF) {
            isEOF = 1;
            break;
        } else if (c == '\n') {
            break;
        }
    }
    if (isEOF)
        return -1;
    return 0;
}

int main(int argc, char *argv[]) {
//    if (argc != 2)
//        return 69;

    int id = 0;
    unsigned int serverId;
    int statusCode = getId(&id, &serverId);
    if (statusCode != 0) {
        return statusCode;
    }

    FILE *data;
    if ((data = fopen("in.txt", "r")) == NULL) {
        return 1;
    }
//    if ((data = fopen(argv[1], "r")) == NULL) {
//        return 1;
//    }

    int timeout = 10;
    char *string;
    char sep;
    char command;
    char object;
    while (statusCode != -1) {
        statusCode = readString(data, &string, " ;\n", &sep);
        if (statusCode > 0) {
            fclose(data);
            return statusCode;
        }
        if (sep == ';') {
            if (strcmp(string, "put") == 0) {
                command = 'p';
            } else if (strcmp(string, "move") == 0) {
                command = 'm';
            } else {
                fclose(data);
                free(string);
                return 2;
            }
        } else if (strcmp(string, "take") == 0) {
            free(string);
            if (statusCode == -1) {
                fclose(data);
                return 2;
            }
            command = 't';
            statusCode = readString(data, &string, ";\n", NULL);
            if (statusCode > 0) {
                free(string);
                fclose(data);
                return statusCode;
            }
            if (strcmp(string, "wolf") == 0) {
                object = 'w';
            } else if (strcmp(string, "goat") == 0) {
                object = 'g';
            } else if (strcmp(string, "cabbage") == 0) {
                object = 'c';
            } else {
                fclose(data);
                free(string);
                return 2;
            }
        } else {
            fclose(data);
            free(string);
            return 2;
        }
        free(string);

        int request = 0;
        char *ptr = (char *) &request;
        memcpy(ptr++, &id, sizeof(char));
        memcpy(ptr++, &command, sizeof(char));
        if (command == 't')
            memcpy(ptr, &object, sizeof(char));

        while (!PostThreadMessageA(serverId, 0, 0, request)) {
            timeout--;
            if (timeout < 1)
                break;
            sleep(1);
        }
        if (timeout < 1) {
            fclose(data);
            return 3;
        }
        timeout = 10;
        statusCode = nextLine(data);
    }

    int request = 0;
    command = 'e';
    char *ptr = (char *) &request;
    memcpy(ptr++, &id, sizeof(char));
    memcpy(ptr, &command, sizeof(char));

    while (!PostThreadMessageA(serverId, 0, 0, request)) {
        timeout--;
        if (timeout < 1)
            break;
        sleep(1);
    }
    if (timeout < 1) {
        fclose(data);
        return 3;
    }
    fclose(data);

    int result;
    statusCode = getResultId(&result, 20, id);
    if (statusCode != 0) {
        printf("kek");
        getchar();
        return statusCode;
    }

    switch (result) {
        case 1:
            printf("Good work\n");
            break;
        case 2:
            printf("Incorrect actions\n");
            break;
        case 3:
            printf("Goat annihilated\n");
            break;
        case 4:
            printf("Cabbage annihilated\n");
            break;
        case 10:
            printf("Triple annihilated\n");
    }
    getchar();
    return 0;
}