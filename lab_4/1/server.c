#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>


#define MUTEX_CONNECT_NAME "connect"
#define MEM_CONNECT_NAME "connectMem"


typedef struct {
    HANDLE mutex;
    HANDLE mapFile;
    LPCTSTR PBuf;
} Connection;

typedef enum {
    EMPTY,
    WOLF,
    GOAT,
    CABBAGE,
    WOLF_GOAT,
    WOLF_CABBAGE,
    GOAT_CABBAGE,
    WOLF_GOAT_CABBAGE,
} Coast;

char ccoast[8][3] = {"e", "w", "g", "c","wg", "wc","gc", "wgc"};
char bboat[4][1] = {"e", "w", "g", "c"};
typedef enum {
    BOAT_EMPTY,
    BOAT_WOLF,
    BOAT_GOAT,
    BOAT_CABBAGE,
} Boat;

typedef struct {
    int id;
    Coast start;
    Coast dest;
    Coast *curCoast;
    Boat boat;
    HANDLE mapFile;
    LPCTSTR PBuf;
    int gameEndStatus; // 0 - continue
} Client;

typedef Client T;
typedef struct {
    T **list;
    int len;
} List;

int listCreate(List **list) {
    *list = (List *) malloc(sizeof(List));
    if ((*list) == NULL) {
        return 1;
    }
    (*list)->len = 0;
    return 0;
}

void listPrint(List *list) {
    printf("------------------------------\n");
    for (int x = 0; x < list->len; x++) {
        printf("%d = \n", list->list[x]->id);
    }
    printf("------------------------------\n");
}

int listAdd(List *list, int id, T **result) {
    if (list == NULL) {
        return 2;
    }
    int key = id;
    int low = 0;
    int high = list->len - 1;
    int isFound = 0;
    int mid;

    while (low <= high) {
        mid = (low + high) >> 1;
        int midVal = list->list[mid]->id;

        if (midVal < key) {
            low = mid + 1;
        } else if (midVal > key) {
            high = mid - 1;
        } else {
            isFound = 1;
            break;
        }
    }
    if (isFound)
        return 2;

    T *new = (T *) malloc(sizeof(T));
    if (new == NULL) {
        return 1;
    }
    new->id = id;

    T **newList;
    if (list->len == 0) {
        newList = (T **) malloc(sizeof(T *));
    } else {
        newList = (T **) realloc(list->list, sizeof(T *) * (list->len + 1));
    }
    if (newList == NULL) {
        free(new);
        return 1;
    }
    list->list = newList;
    // low - index where insert new
    if (low != list->len) {
        memmove(list->list + low + 1, list->list + low, sizeof(list->list) * (list->len - low));
    }
    list->list[low] = new;
    list->len++;
    if (result != NULL)
        *result = new;
    return 0;
}

T *listSearch(List *list, int id) {
    int key = id;
    int low = 0;
    int high = list->len - 1;
    int isFound = 0;
    int mid;

    while (low <= high) {
        mid = (low + high) >> 1;
        int midVal = list->list[mid]->id;

        if (midVal < key) {
            low = mid + 1;
        } else if (midVal > key) {
            high = mid - 1;
        } else {
            isFound = 1;
            break;
        }
    }
    if (isFound)
        return list->list[mid];
    return NULL;
}

int createConnect(Connection *result, char *mutexName, char *mapFileName) {
    if (result == NULL || mutexName == NULL || mapFileName == NULL)
        return 2;
    HANDLE connectMutex;
    HANDLE connectMapFile;
    LPCTSTR connectPBuf;
    connectMutex = CreateMutexA(NULL, FALSE, mutexName);
    if (connectMutex == NULL) {
        return 3;
    }
    connectMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            6,                // maximum object size (low-order DWORD)
            MEM_CONNECT_NAME);                 // name of mapping object

    if (connectMapFile == NULL) {
        CloseHandle(connectMutex);
        return 3;
    }
    connectPBuf = (LPTSTR) MapViewOfFile(connectMapFile,   // handle to map object
                                         FILE_MAP_ALL_ACCESS, // read/write permission
                                         0,
                                         0,
                                         6);

    if (connectPBuf == NULL) {
        CloseHandle(connectMutex);
        CloseHandle(connectMapFile);
        return 3;
    }
    result->mutex = connectMutex;
    result->mapFile = connectMapFile;
    result->PBuf = connectPBuf;
    return 0;
}

void destroyConnect(Connection *connection) {
    UnmapViewOfFile(connection->PBuf);
    CloseHandle(connection->mapFile);
    CloseHandle(connection->mutex);
}

int createClientConnect(T *client) {
    HANDLE connectMapFile;
    LPCTSTR connectPBuf;
    char name[15];
    sprintf(name, "client%d", client->id);
    connectMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            NULL,                    // default security
            PAGE_READWRITE,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            1,                // maximum object size (low-order DWORD)
            name);                 // name of mapping object

    if (connectMapFile == NULL) {
        return 3;
    }
    connectPBuf = (LPTSTR) MapViewOfFile(connectMapFile,   // handle to map object
                                         FILE_MAP_ALL_ACCESS, // read/write permission
                                         0,
                                         0,
                                         1);

    if (connectPBuf == NULL) {
        CloseHandle(connectMapFile);
        return 3;
    }
    int zero = 0;
    memcpy((void *) connectPBuf, &zero, sizeof(char));
    client->mapFile = connectMapFile;
    client->PBuf = connectPBuf;
    return 0;
}

void destroyClientConnect(T *client) {
    UnmapViewOfFile(client->PBuf);
    CloseHandle(client->mapFile);
}

void listDestroy(List *list) {
    if (list == NULL)
        return;
    if (list->len != 0) {
        for (int x = 0; x < list->len; x++) {
            destroyClientConnect(list->list[x]);
            free(list->list[x]);
        }
        free(list->list);
    }
    free(list);
}

int isContainCoast(Coast coast, char object) {
    switch (object) {
        case 'w': {
            if (coast == WOLF || coast == WOLF_CABBAGE || coast == WOLF_GOAT ||
                coast == WOLF_GOAT_CABBAGE)
                return 1;
            return 0;
        }
        case 'g': {
            if (coast == GOAT || coast == GOAT_CABBAGE || coast == WOLF_GOAT ||
                coast == WOLF_GOAT_CABBAGE)
                return 1;
            return 0;
        }
        case 'c': {
            if (coast == CABBAGE || coast == GOAT_CABBAGE || coast == WOLF_CABBAGE ||
                coast == WOLF_GOAT_CABBAGE)
                return 1;
            return 0;
        }
    }
    return 0;
}

Boat takeToBoat(char object) {
    switch (object) {
        case 'w': return BOAT_WOLF;
        case 'g': return BOAT_GOAT;
        case 'c': return BOAT_CABBAGE;
    }
    return BOAT_EMPTY;
}

Coast putToCoast(Coast coast, Boat object) {
    if (object == BOAT_EMPTY)
        return coast;
    switch (object) {
        case BOAT_WOLF: {
            switch (coast) {
                case EMPTY: return WOLF;
                case CABBAGE: return WOLF_CABBAGE;
                case GOAT: return WOLF_GOAT;
                case GOAT_CABBAGE: return WOLF_GOAT_CABBAGE;
            }
        }
        case BOAT_GOAT: {
            switch (coast) {
                case EMPTY: return GOAT;
                case CABBAGE: return GOAT_CABBAGE;
                case WOLF: return WOLF_GOAT;
                case WOLF_CABBAGE: return WOLF_GOAT_CABBAGE;
            }
        }
        case BOAT_CABBAGE: {
            switch (coast) {
                case EMPTY: return CABBAGE;
                case GOAT: return GOAT_CABBAGE;
                case WOLF: return WOLF_CABBAGE;
                case WOLF_GOAT: return WOLF_GOAT_CABBAGE;
            }
        }
    }
    return coast;
}

Coast takeFromCoast(Coast coast, char object) {
    if (!isContainCoast(coast, object))
        return coast;
    switch (object) {
        case 'w': {
            switch (coast) {
                case WOLF: return EMPTY;
                case WOLF_CABBAGE: return CABBAGE;
                case WOLF_GOAT: return GOAT;
                case WOLF_GOAT_CABBAGE: return GOAT_CABBAGE;
            }
        }
        case 'g': {
            switch (coast) {
                case GOAT: return EMPTY;
                case GOAT_CABBAGE: return CABBAGE;
                case WOLF_GOAT: return WOLF;
                case WOLF_GOAT_CABBAGE: return WOLF_CABBAGE;
            }
        }
        case 'c': {
            switch (coast) {
                case CABBAGE: return EMPTY;
                case GOAT_CABBAGE: return GOAT;
                case WOLF_CABBAGE: return WOLF;
                case WOLF_GOAT_CABBAGE: return WOLF_GOAT;
            }
        }
    }
    return coast;
}

void checkConnectRequest(int *id, Connection *connection) {
    int status;
    memcpy(&status, (void *) connection->PBuf, sizeof(char));
    if (status == 1) {
        (*id)++;
        memcpy((void *) (connection->PBuf + 1), id, sizeof(char));
        status = 0;
        memcpy((void *) connection->PBuf, &status, sizeof(char));
    }
}

int processMessageRequest(List *clients, int id, char command, char object) {
    T *client = listSearch(clients, id);
    if (client == NULL) {
        int statusCode = listAdd(clients, id, &client);
        if (statusCode != 0)
            return statusCode;
        statusCode = createClientConnect(client);
        if (statusCode != 0)
            return statusCode;
        client->curCoast = &(client->start);
        client->start = WOLF_GOAT_CABBAGE;
        client->dest = EMPTY;
        client->boat = BOAT_EMPTY;
        client->gameEndStatus = 0;
    }
    if (command != 'e' && client->gameEndStatus)
        return 0;
    switch (command) {
        case 't': {
            if (!isContainCoast(*(client->curCoast), object) || client->boat != BOAT_EMPTY) {
                client->gameEndStatus = 2;
                return 0;
            }
            *(client->curCoast) = takeFromCoast(*(client->curCoast), object);
            client->boat = takeToBoat(object);
            break;
        }
        case 'p': {
            if (client->boat == BOAT_EMPTY) {
                client->gameEndStatus = 2;
                return 0;
            }
            *(client->curCoast) = putToCoast(*(client->curCoast), client->boat);
            client->boat = BOAT_EMPTY;
            break;
        }
        case 'm': {
            switch (*(client->curCoast)) {
                case WOLF_GOAT_CABBAGE:
                    client->gameEndStatus = 10;
                    return 0;
                case WOLF_GOAT:
                    client->gameEndStatus = 3;
                    return 0;
                case GOAT_CABBAGE:
                    client->gameEndStatus = 4;
                    return 0;
            }
            if (client->curCoast == &(client->start)) {
                client->curCoast = &(client->dest);
            } else {
                client->curCoast = &(client->start);
            }
            break;
        }
        case 'e': {  // end
            if (!client->gameEndStatus) {
                if (client->dest == WOLF_GOAT_CABBAGE)
                    client->gameEndStatus = 1;
                else
                    client->gameEndStatus = 2;
            }
            memcpy((void *) client->PBuf, &(client->gameEndStatus), sizeof(char));
        }
    }
    return 0;
}

void createMessageQueue() {
    MSG message;
    PeekMessage(&message, NULL, WM_USER, WM_USER, PM_NOREMOVE);
}

int checkMessageRequest(List *clients) {
    MSG message;
    if (!PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        return 0;
    }
    int clientId = 0;
    char command;
    char object = ' ';
    char *ptr = (char *) &message.lParam;
    memcpy(&clientId, ptr++, sizeof(char));
    memcpy(&command, ptr++, sizeof(char));
    if (command == 't')
        memcpy(&object, ptr, sizeof(char));
    return processMessageRequest(clients, clientId, command, object);
}

int main(int argc, char *argv[]) {
    int statusCode = 0;
    List *clients;
    statusCode = listCreate(&clients);
    if (statusCode != 0)
        return statusCode;

    Connection connectRequest;
    statusCode = createConnect(&connectRequest, MUTEX_CONNECT_NAME, MEM_CONNECT_NAME);
    if (statusCode != 0) {
        return statusCode;
    }

    unsigned int threadId = GetCurrentThreadId();
    memcpy((void *) (connectRequest.PBuf + 2), &threadId, sizeof(int));

    createMessageQueue();

    int id = 0;
    int workTime = 30;
    while (workTime) {
        if (id < 127)
            checkConnectRequest(&id, &connectRequest);

        checkMessageRequest(clients);

        if (clients->len) {
            printf("%d | %s | %s | %c\n", clients->list[0]->gameEndStatus, ccoast[clients->list[0]->start],
                   ccoast[clients->list[0]->dest], bboat[clients->list[0]->boat][0]);
        }

        sleep(1);
        workTime--;
    }

    listDestroy(clients);
    destroyConnect(&connectRequest);
    return 0;
}
