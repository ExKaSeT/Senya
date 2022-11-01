#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

void freeAll(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int x = 0; x < count; x++) {
        free(va_arg(args, void *));
    }
}

typedef struct {
    char *data;
    unsigned int len;
} String;

void printString(String *string) {
    for (int x = 0; x < string->len; x++) {
        putchar(string->data[x]);
    }
}

int createString(String **result, const char *string) {
    int len = 0;
    char *data = NULL;
    String *res;
    if (string != NULL) {
        while (string[len] != '\0')
            len++;
        if (len == 0)
            return 2;
        data = (char *) malloc(sizeof(char) * len);
        if (data == NULL) {
            return 1;
        }
        memcpy(data, string, len * sizeof(char));
    }
    res = (String *) malloc(sizeof(String));
    if (res == NULL) {
        if (data != NULL)
            free(data);
        return 1;
    }
    res->len = len;
    res->data = data;
    *result = res;
    return 0;
}

void clearString(String *string) {
    if (string->len == 0)
        return;
    free(string->data);
    string->len = 0;
}

void freeString(String *string) {
    clearString(string);
    free(string);
}

int compareStrings(String *str1, String *str2, int (*comparator)(char, char)) {
    for (int x = 0; x < (str1->len > str2->len ? str2->len : str1->len); x++) {
        int code = comparator(str1->data[x], str2->data[x]);
        if (code > 0) {
            return 1;
        } else if (code < 0) {
            return -1;
        }
    }
    if (str1->len > str2->len)
        return 1;
    if (str1->len < str2->len)
        return -1;
    return 0;
}

int copyString(String *string, String *destination) {
    clearString(destination);
    char *data = (char *) malloc(sizeof(char) * string->len);
    if (data == NULL) {
        return 1;
    }
    memcpy(data, string->data, string->len * sizeof(char));
    destination->len = string->len;
    destination->data = data;
    return 0;
}

int concatStrings(String **result, int count, ...) {
    String **strings = (String **) malloc(sizeof(String *) * count);
    if (strings == NULL)
        return 1;
    va_list args;
    va_start(args, count);
    unsigned int resLen = 0;
    for (int x = 0; x < count; x++) {
        strings[x] = va_arg(args, String *);
        resLen += strings[x]->len;
    }
    char *data = (char *) malloc(sizeof(char) * resLen);
    if (data == NULL) {
        free(strings);
        return 1;
    }
    String *res = (String *) malloc(sizeof(String));
    if (res == NULL) {
        freeAll(2, data, strings);
        return 1;
    }
    resLen = 0;
    for (int x = 0; x < count; x++) {
        memcpy(data + resLen, strings[x]->data, strings[x]->len * sizeof(char));
        resLen += strings[x]->len;
    }
    res->len = resLen;
    res->data = data;
    *result = res;
    free(strings);
    return 0;
}

int duplicateString(String **result, String *string) {
    switch (createString(result, NULL)) {
        case 1:
            return 1;
        case 2:
            return 2;
    }
    switch (copyString(string, *result)) {
        case 1:
            freeString(*result);
            return 3;
    }
    return 0;
}

int comp(char a, char b) {
    return a - b;
}

typedef struct {
    String *city;
    String *street;
    int house;
    int flat;
    String *index;
} Address;

typedef struct tm Date;

typedef struct {
    Address *sender;
    Address *getter;
    double weight;
    String *id;
    Date creationTime;
    Date deliveryTime;
} Mail;

typedef struct {
    Address *address;
    Mail **mails;
    int mailCount;
} Post;

int isLetter(char c) {
    return (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

int isNum(char c) {
    return ('0' <= c && c <= '9');
}

int readString(char **result, char *separators, int (*isCorrectChar)(char)) {
    int resCapacity = 1;
    char *res = (char *) malloc(sizeof(char) * resCapacity);
    if (res == NULL)
        return 1;
    char c;
    int count = 0;
    while (1) {
        c = getchar();
        int x = 0;
        int isBreak = 0;
        while (separators[x] != '\0') {
            if (c == separators[x]) {
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
        if (isCorrectChar != NULL && !isCorrectChar(c)) {
            free(res);
            return 2;
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
    return 0;
}

int parseInt(const char *string) { // correct number in str
    int x = 0;
    while (string[x] == '0')
        x++;
    if (string[x] == '\0')
        return 0;
    int num = (string[x] - '0');
    x++;
    while (string[x] != '\0') {
        num = num * 10 + (string[x] - '0');
        x++;
    }
    return num;
}

int createAddress(Address **result, char *city, char *street, int house, int flat, char *index) {
    if (house < 1 || flat < 1)
        return 2;
    Address *address = (Address *) malloc(sizeof(Address));
    if (address == NULL) {
        return 1;
    }
    int statusCode;
    String *string;
    statusCode = createString(&string, city);
    if (statusCode != 0) {
        free(address);
        return statusCode;
    }
    address->city = string;
    statusCode = createString(&string, street);
    if (statusCode != 0) {
        freeAll(2, address->city, address);
        return statusCode;
    }
    address->street = string;
    statusCode = createString(&string, index);
    if (statusCode != 0) {
        freeAll(3, address->city, address->street, address);
        return statusCode;
    }
    address->index = string;
    address->house = house;
    address->flat = flat;
    *result = address;
    return 0;
}

void freeAddress(Address *address) {
    freeAll(4, address->city, address->street, address->index, address);
}

void freeMail(Mail *mail) {
    freeAddress(mail->getter);
    freeAddress(mail->sender);
    freeString(mail->id);
    free(mail);
}

int isCorrectDouble(char *string, double *result) {
    char *endPtr;
    errno = 0;
    *result = strtod(string, &endPtr);
    if (errno != 0 || *endPtr != '\0')
        return 0;
    return 1;
}

Date parseDate(const char *date) { // correct date
    int day;
    int x = 0;
    day = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    int month;
    x = 3;
    month = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    int year;
    x = 6;
    year = (date[x] - '0');
    x++;
    while (x != 10) {
        year = year * 10 + (date[x] - '0');
        x++;
    }

    int hours;
    x = 11;
    hours = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    int minutes;
    x = 14;
    minutes = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    int seconds;
    x = 17;
    seconds = (date[x] == '0') ? (date[x + 1] - '0') : (date[x] - '0') * 10 + (date[x + 1] - '0');

    Date d;
    d.tm_hour = hours;
    d.tm_min = minutes;
    d.tm_sec = seconds;
    d.tm_year = year - 1900;
    d.tm_mon = month - 1;
    d.tm_mday = day;
    return d;
}

int compareDate(Date *date1, Date *date2) {
    if (date1->tm_year > date2->tm_year) {
        return 1;
    } else if (date1->tm_year < date2->tm_year) {
        return -1;
    }
    if (date1->tm_mon > date2->tm_mon) {
        return 1;
    } else if (date1->tm_mon < date2->tm_mon) {
        return -1;
    }
    if (date1->tm_mday > date2->tm_mday) {
        return 1;
    } else if (date1->tm_mday < date2->tm_mday) {
        return -1;
    }
    if (date1->tm_hour > date2->tm_hour) {
        return 1;
    } else if (date1->tm_hour < date2->tm_hour) {
        return -1;
    }
    if (date1->tm_min > date2->tm_min) {
        return 1;
    } else if (date1->tm_min < date2->tm_min) {
        return -1;
    }
    if (date1->tm_sec > date2->tm_sec) {
        return 1;
    } else if (date1->tm_sec < date2->tm_sec) {
        return -1;
    }
    return 0;
}

int isCorrectDate(const char *string) {
    int day, month, year, hours, minutes, seconds;
    int dotCount = 0;
    // проверка строки (даты)
    if (string[10] != ' ')
        return 0;
    int x = 0;
    while (x != 10) {
        if (string[x] == '.') {
            if (x != 2 && x != 5)
                return 0;
            dotCount++;
        } else if (!isNum(string[x])) {
            return 0;
        }
        x++;
    }
    if (dotCount != 2)
        return 0;
    if (string[6] == '0') // первая цифра года
        return 0;
    // проверка строки (времени)
    x = 11;
    dotCount = 0;
    while (string[x] != '\0') {
        if (string[x] == ':') {
            if (x != 13 && x != 16)
                return 0;
            dotCount++;
        } else if (!isNum(string[x])) {
            return 0;
        }
        x++;
    }
    if (x != 19)
        return 0;
    if (dotCount != 2)
        return 0;
    // ---------------------------------------------
    Date date = parseDate(string);
    day = date.tm_mday;
    month = date.tm_mon + 1;
    year = date.tm_year + 1900;
    hours = date.tm_hour;
    minutes = date.tm_min;
    seconds = date.tm_sec;
    if (hours < 0 || hours > 23)
        return 0;
    if (minutes < 0 || minutes > 59)
        return 0;
    if (seconds < 0 || seconds > 59)
        return 0;
    if (day < 1)
        return 0;
    if (month < 1 || month > 12)
        return 0;
    if (year < 2000)
        return 0;
    if (month == 2) {
        int leap = 0;
        if (((year % 4 == 0) && (year % 100 != 0)) || year % 400 == 0) {
            leap = 1;
        }
        if (!leap && day <= 28) {
            return 1;
        } else if (leap && day <= 29) {
            return 1;
        }
        return 0;
    }
    if (month < 8) {
        if (day <= month % 2 + 30)
            return 1;
        return 0;
    } else {
        if (day <= 31 - month % 2)
            return 1;
        return 0;
    }
}

int readMail(Mail **result) {
    char *city, *street, *houseString, *flatString, *index, *weightString, *id, *creationTime, *deliveryTime;
    int house, flat;
    double weight;

    int statusCode;
    statusCode = readString(&city, "\t", isLetter);
    if (statusCode != 0) {
        return statusCode;
    }
    statusCode = readString(&street, "\t", isLetter);
    if (statusCode != 0) {
        free(city);
        return statusCode;
    }
    statusCode = readString(&houseString, "\t", isNum);
    if (statusCode != 0) {
        freeAll(2, city, street);
        return statusCode;
    }
    house = parseInt(houseString);
    free(houseString);
    statusCode = readString(&flatString, "\t", isNum);
    if (statusCode != 0) {
        freeAll(2, city, street);
        return statusCode;
    }
    flat = parseInt(flatString);
    free(flatString);
    statusCode = readString(&index, "\t", isNum);
    if (statusCode != 0) {
        freeAll(2, city, street);
        return statusCode;
    }
    if (strlen(index) != 6) {
        freeAll(3, city, street, index);
        return 4;
    }

    Address *address;
    statusCode = createAddress(&address, city, street, house, flat, index);
    if (statusCode != 0) {
        freeAll(4, city, street, index);
        return statusCode;
    }
    freeAll(3, city, street, index);
    Mail *mail = (Mail *) malloc(sizeof(Mail));
    if (mail == NULL) {
        freeAddress(address);
        return 1;
    }
    mail->sender = address;

    statusCode = readString(&city, "\t", isLetter);
    if (statusCode != 0) {
        free(mail);
        freeAddress(address);
        return statusCode;
    }
    statusCode = readString(&street, "\t", isLetter);
    if (statusCode != 0) {
        freeAll(2, city, mail);
        freeAddress(address);
        return statusCode;
    }
    statusCode = readString(&houseString, "\t", isNum);
    if (statusCode != 0) {
        freeAll(3, city, street, mail);
        freeAddress(address);
        return statusCode;
    }
    house = parseInt(houseString);
    free(houseString);
    statusCode = readString(&flatString, "\t", isNum);
    if (statusCode != 0) {
        freeAll(3, city, street, mail);
        freeAddress(address);
        return statusCode;
    }
    flat = parseInt(flatString);
    free(flatString);
    statusCode = readString(&index, "\t", isNum);
    if (statusCode != 0) {
        freeAll(3, city, street, mail);
        freeAddress(address);
        return statusCode;
    }
    if (strlen(index) != 6) {
        freeAll(4, city, street, index, mail);
        freeAddress(address);
        return 4;
    }

    statusCode = createAddress(&address, city, street, house, flat, index);
    if (statusCode != 0) {
        freeAll(4, city, street, index, mail);
        return statusCode;
    }
    freeAll(3, city, street, index);

    mail->getter = address;


    statusCode = readString(&weightString, "\t", NULL);
    if (statusCode != 0) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        free(mail);
        return statusCode;
    }
    if (!isCorrectDouble(weightString, &weight)) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(2, mail, weightString);
        return 6;
    }
    free(weightString);
    statusCode = readString(&id, "\t", isNum);
    if (statusCode != 0) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        free(mail);
        return statusCode;
    } else if (strlen(id) != 14) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(2, mail, id);
        return 6;
    }
    statusCode = readString(&creationTime, "\t", NULL);
    if (statusCode != 0) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(2, mail, id);
        return statusCode;
    } else if (!isCorrectDate(creationTime)) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(2, mail, id);
        return 5;
    }
    statusCode = readString(&deliveryTime, "\n", NULL);
    if (statusCode != 0) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(3, mail, id, creationTime);
        return statusCode;
    } else if (!isCorrectDate(deliveryTime)) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(4, mail, id, creationTime, deliveryTime);
        return 5;
    }

    mail->weight = weight;
    mail->creationTime = parseDate(creationTime);
    mail->deliveryTime = parseDate(deliveryTime);
    freeAll(2, creationTime, deliveryTime);
    String *stringId;
    statusCode = createString(&stringId, id);
    if (statusCode != 0) {
        freeAddress(mail->sender);
        freeAddress(mail->getter);
        freeAll(2, mail, id);
        return statusCode;
    }
    free(id);
    mail->id = stringId;
    *result = mail;
    return 0;
}

void printTime(Date *date) {
    char string[21];
    strftime(string, 21, "%d.%m.%Y %H:%M:%S ", date);
    printf("%s", string);
}

int addMail(Post *post, Mail *mail) {
    Mail **newList;
    if (post->mailCount != 0) {
        newList = (Mail **) realloc(post->mails, (post->mailCount + 1) * sizeof(Mail *));
    } else {
        newList = (Mail **) malloc(sizeof(Mail *));
    }
    if (newList == NULL) {
        return 1;
    }
    post->mails = newList;
    post->mails[post->mailCount] = mail;
    post->mailCount++;
    return 0;
}

int deleteMail(Post *post, int index) {
    if (index < 0 || index >= post->mailCount)
        return 1;
    if (post->mailCount == 0)
        return 2;
    if (post->mailCount == 1) {
        freeMail(post->mails[0]);
        post->mailCount--;
        return 0;
    }
    if (post->mailCount == index - 1) {
        freeMail(post->mails[index]);
    } else {
        memcpy(post->mails[index], post->mails[index + 1], post->mailCount - 1 - index);
    }
    post->mailCount--;
    Mail **newList = (Mail **) realloc(post->mails, post->mailCount * sizeof(Mail *));
    if (newList == NULL) {
        return 3;
    }
    post->mails = newList;
    return 0;
}

int mailComparator(Mail **m1, Mail **m2) {
    Mail *mail1 = *m1;
    Mail *mail2 = *m2;
    int res = compareStrings(mail1->getter->index, mail2->getter->index, comp);
    if (res > 0) {
        return 1;
    } else if (res < 0) {
        return -1;
    }
    res = compareStrings(mail1->id, mail2->id, comp);
    if (res > 0) {
        return 1;
    } else if (res < 0) {
        return -1;
    }
    return 0;
}

void sortMails(Post *post) {
    if (post->mailCount < 2)
        return;
    qsort(post->mails, post->mailCount, sizeof(Mail *),
          (int (*)(const void *, const void *)) mailComparator);
}

void printMail(Mail *mail) {
    printf("\n-----------------------------------\n");
    printf("Sender: ");
    printString(mail->sender->city);
    printf(" ");
    printString(mail->sender->street);
    printf(" %d-%d ", mail->sender->house, mail->sender->flat);
    printString(mail->sender->index);
    printf("\nGetter: ");
    printString(mail->getter->city);
    printf(" ");
    printString(mail->getter->street);
    printf(" %d-%d ", mail->getter->house, mail->getter->flat);
    printString(mail->getter->index);
    printf("\nID:");
    printString(mail->id);
    printf("\ncreated ");
    printTime(&(mail->creationTime));
    printf("\ndelivered ");
    printTime(&(mail->deliveryTime));
    printf("\n-----------------------------------\n");
}

void printMails(Mail **mails, int count) {
    for (int x = 0; x < count; x++) {
        printMail(mails[x]);
    }
}

Mail *searchById(Mail **mails, int count, String *id) {
    for (int x = 0; x < count; x++) {
        if (compareStrings(mails[x]->id, id, comp) == 0)
            return mails[x];
    }
    return NULL;
}

int getDeliveredMails(Mail ***result, int *resLen, Mail **mails, int len) {
    int count = 0;
    int resCapacity = 1;
    Mail *mail;
    Mail **res = (Mail **) malloc(sizeof(Mail *) * resCapacity);
    if (res == NULL)
        return 1;
    for (int x = 0; x < len; x++) {
        mail = mails[x];
        if (count == resCapacity) {
            resCapacity *= 2;
            Mail **newRes = (Mail **) realloc(res, sizeof(Mail *) * resCapacity);
            if (newRes == NULL) {
                free(res);
                return 2;
            }
            res = newRes;
        }

        const time_t timer = time(NULL);
        Date *current = localtime(&timer);

        if (compareDate(&(mail->deliveryTime), current) == 1)
            continue;

        res[count] = mail;
        count++;
    }
    if (resCapacity != count) {
        Mail **newRes = (Mail **) realloc(res, sizeof(Mail *) * count);
        if (newRes == NULL) {
            free(res);
            return 3;
        }
        res = newRes;
    }
    qsort(res, count, sizeof(Mail *),
          (int (*)(const void *, const void *)) mailComparator);

    *result = res;
    *resLen = count;
    return 0;
}

void freePost(Post *post) {
    if (post->mailCount != 0) {
        for (int x = 0; x < post->mailCount; x++) {
            freeMail(post->mails[x]);
        }
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);
    Mail *mail;
    Post post;
    post.mailCount = 0;
    int statusCode = 0;
//    printf("%d\n", readMail(&mail));
//    printf("%d\n", addMail(&post, mail));
//    printf("%d\n", readMail(&mail));
//    printf("%d\n", addMail(&post, mail));
//    sortMails(&post);
//    printMails(post.mails, post.mailCount);
//    String *str;
//    createString(&str, "12345678901234");
//    printMail(searchById(post.mails, post.mailCount, str));


    int action;
    do {
        printf("\n1 - add mail\t2 - delete mail\t3 - search by ID\n");
        printf("4 - sort\t5 - print mails\t6 - find delivered\n69 - exit\n");
        scanf("%d", &action);
        getchar();
        if (action != 69 && (action < 1 || action > 6)) {
            return 666;
        }
        switch (action) {
            case 1: {
                getchar();
                statusCode = readMail(&mail);
                if (statusCode != 0) {
                    freePost(&post);
                    return statusCode;
                }
                statusCode = addMail(&post, mail);
                if (statusCode != 0) {
                    freePost(&post);
                    freeMail(mail);
                    return statusCode;
                }
                break;
            }
            case 2: {
                printf("Enter index: ");
                int index;
                scanf("%d", &index);
                statusCode = deleteMail(&post, index);
                if (statusCode != 0) {
                    freePost(&post);
                    return statusCode;
                }
                break;
            }
            case 3: {
                char id[15];
                String *str;
                printf("Enter id: ");
                scanf("%s", id);
                id[14] = '\0';
                createString(&str, id);
                printMail(searchById(post.mails, post.mailCount, str));
                freeString(str);
                break;
            }
            case 4: {
                sortMails(&post);
                break;
            }
            case 5: {
                if (post.mailCount == 0)
                    return 999;
                printMails(post.mails, post.mailCount);
                break;
            }
            case 6: {
                Mail **mail1;
                int len;
                statusCode = getDeliveredMails(&mail1, &len, post.mails, post.mailCount);
                if (statusCode != 0) {
                    freePost(&post);
                    return statusCode;
                }
                printMails(mail1, len);
                free(mail1);
                break;
            }
        }
    } while (action != 69);
    return 0;
}