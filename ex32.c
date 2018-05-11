#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#define CONFIGURATION 3
#define PATH 1000
#define NAME 17
#define FAIL -1
#define STUDENTS 100


typedef struct Student {
    char name[STUDENTS];
    int grade;
    char resultCompare[NAME];
} Student;

void makeConfigurationFile(char info[CONFIGURATION][STUDENTS], char *path, struct dirent *pDirent) {
    DIR *pDir;
    int i = 0;
    int length = 0;
    int flag = 0;
    ssize_t charToRead;
    int fd = open(path, O_RDONLY);
    char buffer[1];
    while ((charToRead = read(fd, buffer, sizeof(buffer))) != 0) {
        if (!flag) {
            info[i][length] = *buffer;
            flag = 1;
            length++;
        }
        while (buffer[0] != '\n') {
            charToRead = read(fd, buffer, sizeof(buffer));
            info[i][length] = *buffer;
            length++;
        }
        info[i][length - 1] = '\0';
        i++;
        flag = 0;
        length = 0;
    }
}


void compareFiles(char *correctOutput, char *outputFile, Student *oneStudent) {
    pid_t pid;
    pid = fork();
    char *outPutFileTxt;

    //outPutFileTxt= strcat(outputFile,".txt");

    char *arguments[5];

    arguments[0] = "./comp.out";
    arguments[1] = "comp.out";
    strcpy(arguments[2], correctOutput);
    strcpy(arguments[3], outPutFileTxt);
    arguments[4] = NULL;
    if (pid == 0) {
        int result = execvp("./comp.out", arguments);
        switch (result) {
            case 1: {
                oneStudent->grade = 60;
                strcpy(oneStudent->resultCompare, "BAD_OUTPUT");
                break;
            }
            case 2: {
                oneStudent->grade = 80;
                strcpy(oneStudent->resultCompare, "SIMILAR_OUTPUT");
                break;
            }
            case 3: {
                oneStudent->grade = 100;
                strcpy(oneStudent->resultCompare, "GREAT_JOB");
                break;
            }
            default:
                break;
        }

    } else {

    }

}

void execute(char *path, char *inputFile, char *outputFile, Student *oneStudent) {
    char *outPutFileTxt;
    int in, out;
    outPutFileTxt = strcat(outputFile, ".txt");
    in = open(inputFile, O_RDWR);
    out = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (in == FAIL || out == FAIL) {
        fprintf(stderr, "Can not open the input/output files");
        exit(FAIL);
    }
    //set the inputFile is the input for the cFile, as well as the output cFile
    dup2(in, 0);
    dup2(out, 1);
    pid_t pid;
    pid = fork();
    char *arguments[3];
    arguments[0] = "a.out";
    arguments[1] = inputFile;
    arguments[2] = NULL;

    if (pid == 0) {
        if (execvp(arguments[0], arguments) == FAIL) {
            fprintf(stderr, "Can not run the exe");
            exit(FAIL);
        }
    } else {// fathers process
        pid_t pidProcess = waitpid(pid, NULL, WNOHANG);
        if (pidProcess == FAIL)
            fprintf(stderr, "Can not compile this c file");
        compareFiles(outPutFileTxt, outputFile, oneStudent);
    }

}


void compile(char *cFile, char *inputFile, char *outputFile, char *path, Student *oneStudent) {
    int in, out;
    char *arguments[3];

    arguments[0] = "gcc";
    arguments[1] = cFile;
    arguments[2] = NULL;
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        int result = execvp(arguments[0], arguments);
        if (result == FAIL) {
            oneStudent->grade = 0;
            strcpy(oneStudent->resultCompare, "COMPILATION_ERROR");
            fprintf(stderr, "Can not compile this c file");
            exit(FAIL);
        }
    } else {// fathers process
        pid_t pidProcess = waitpid(pid, NULL, WNOHANG);
        if (pidProcess == FAIL)
            fprintf(stderr, "Can not wait for this c file");

        // execute the file we have compiled
        execute(path, inputFile, outputFile, oneStudent);
    }


}


int
handleDirectory(char directoryName[STUDENTS], char *inputFile, char *outputFile, char *path, Student *student, int i) {
    struct dirent *pDirent;
    DIR *pDir;
    //int i = 0;
    int length = 0;

    char insideDir[STUDENTS], upDirectory[STUDENTS];
    if ((pDir = opendir(directoryName)) == NULL)
        exit(1);

    while (directoryName[length] != '\0') {
        length++;
    }

    while ((pDirent = readdir(pDir)) != NULL) {

        if ((directoryName[length - 1] != 'c') && (directoryName[length - 2] != '.') && pDirent->d_type == DT_DIR) {
            //firstCCheck = 0;

            if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
                continue;
            }
            strcpy(insideDir, directoryName);
            strcat(insideDir, "/");
            strcat(insideDir, pDirent->d_name);
            handleDirectory(directoryName, inputFile, outputFile, directoryName, student, i);

        } else if (pDirent->d_type == DT_REG) {
            if (strcmp(strrchr(pDirent->d_name, '.'), ".c") == 0)
                //else if ((directoryName[length-1]=='c' )&&(directoryName[length-2]=='.') && pDirent->d_type == DT_REG) {

                strcpy(student->name, pDirent->d_name);//updating the name of the student
            strcat(directoryName, "/");
            strcat(directoryName, pDirent->d_name);
            compile(directoryName, inputFile, outputFile, path, student);

            // it is not the first time and there is not a c file
        } else {
            strcpy(student->name, pDirent->d_name);//updating the name of the student
            student->grade = 0;
            strcpy(student->resultCompare, "NO_C_FILE");
        }
        //i++;

    }
    //return i;//signify how many students their are
}


/*
char *resultCompare(int option) {
    switch (option) {
        case 1:
            return "NO_C_FILE";
        case 2:
            return "COMPILATION_ERROR";
        case 3:
            return "TIMEOUT";
        case 4:
            return "BAD_OUTPUT";
        case 5:
            return "SIMILAR_OUTPUT";
        case 6:
            return "GREAT_JOB";
        default:
            break;
    }

}
*/

void makeResultCSVFile(Student **students, int numOfStudents) {
    int index;
    int fdCsv = open("results.csv", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (fdCsv == FAIL) {
        fprintf(stderr, "Can not create result.csv file");
        exit(FAIL);
    } else {
        char line[STUDENTS];
        for (index = 0; index < numOfStudents; index++)
            memset(line, '\0', STUDENTS);
        snprintf(line, sizeof(line), "%s,%d,%s\n", students[index]->name, students[index]->grade,
                 students[index]->resultCompare);
        write(fdCsv, line, STUDENTS);
        close(fdCsv);
    }
}

int main(int argc, char **argv) {

    if (argc != 2)
        return -1;
    else {
        DIR *pDir;
        int file1;
        struct dirent *pDirent;
        char *dir, *inputFile, *outputFile;
        char info[CONFIGURATION][STUDENTS];
        char path[PATH], tempDir[STUDENTS];
        char correctOutput[17] = "correctOutput.txt";
        Student *students[STUDENTS];
        int numOfStudents = 0;
        int i = 0;
        int j;
        makeConfigurationFile(info, argv[1], pDirent);

        dir = info[0];
        inputFile = info[1];
        outputFile = info[2];
        strcpy(path, dir);//starting concating the path for compilation

        if ((pDir = opendir(info[0])) == NULL)
            exit(FAIL);

        while ((pDirent = readdir(pDir)) != NULL) {
            if (strcmp(pDirent->d_name, "..") == 0 || strcmp(pDirent->d_name, ".") == 0) {
                continue;
            }
            strcpy(tempDir, info[0]);
            strcat(tempDir, "/");
            strcat(tempDir, pDirent->d_name);
            //strcpy(students[i]->name,'\0');
            students[i] = (Student *) malloc(sizeof(Student));
            numOfStudents = handleDirectory(tempDir, info[1], info[2], path, students[i], i);
            i++;
        }

        makeResultCSVFile(students, numOfStudents);

        for (j = 0; j < i; j++) {
            free(students[i]);
        }

        return 0;
    }
}


