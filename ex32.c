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
#define STUDENTS FILENAME_MAX
#define STDERR 2
#define ERROR "Error in system call\n"
#define SIZEERROR 21



typedef struct Student {
    char name[STUDENTS];
    int grade;
    char resultCompare[NAME];
} Student;

/**
 * this function read from the file all the prarms
 * @param info is the array that will hold all the params
 * @param path of the configuration file that should be opened.
 */
void makeConfigurationFile(char info[CONFIGURATION][STUDENTS], char *path) {
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

/**
 * this functions send the two txt files to the program from the last assignment to check how equal are they.
 * @param outputFile is the out put file that i wrote to after gcc and a.out
 * @param oneStudent is the current student that i am checking.
 */
void compareFiles(char *outputFile, Student *oneStudent) {
    pid_t pid;
    pid = fork();
    char *outPutFileTxt;

    if (pid == 0) {
        char *arguments[NAME];

        arguments[0] = "./comp.out";
        arguments[1] = outputFile;
        arguments[2] = "myFile.txt";
        arguments[3] = NULL;
        execvp(arguments[0], arguments);
    } else {
        int result;
        waitpid(pid, &result, WCONTINUED);
        //result = WEXITSTATUS(result);
        switch (WEXITSTATUS(result)) {
            case 0:{
                oneStudent->grade = 60;
                strcpy(oneStudent->resultCompare, "BAD_OUTPUT");
                break;
            }
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

    }

}

/**
 * this function run the program after compiling it.
 * @param path is the path of the exe file
 * @param inputFile is the input for the program
 * @param outputFile is the out put file that we write to
 * @param oneStudent is the current dtudent to handle.
 */
void execute(char *path, char *inputFile, char *outputFile, Student *oneStudent) {
    int in, out;
    in = open(inputFile, O_RDWR);
    out = open("myFile.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (in == FAIL || out == FAIL) {
        write(STDERR, ERROR, SIZEERROR);
        exit(FAIL);
    }
    //set the inputFile is the input for the cFile, as well as the output cFile
    if (dup2(in, 0) == FAIL) {
        write(STDERR, ERROR, SIZEERROR);
        exit(FAIL);
    }

    if (dup2(out, 1) == FAIL) {
        write(STDERR, ERROR, SIZEERROR);
        exit(FAIL);
    }
    pid_t pid;
    char *arguments[NAME];
    arguments[0] = "./a.out";
    arguments[1] = NULL;
    //TODO check fork every where!@#$!%#@^%^#@#@$%
    pid = fork();

    if (pid == 0) {
        if (execvp(arguments[0], arguments) == FAIL) {
            write(STDERR, ERROR, SIZEERROR);
            exit(FAIL);
        }
    } else {// fathers process
        sleep(5);
        pid_t pidProcess = waitpid(pid, NULL, WNOHANG);
        if (!pidProcess) {
            //TODO check name of the time out maybe use the strtok

            oneStudent->grade = 0;
            strcpy(oneStudent->resultCompare, "TIMEOUT");
        }
        if (pidProcess == FAIL)
            write(STDERR, ERROR, SIZEERROR);
        compareFiles(outputFile, oneStudent);
    }
    close(in);
    close(out);
}


/**
 * thi function compile the c file after finding out that there is a c file.
 * @param cFile is the path for the c file.
 * @param inputFile is the input for the program
 * @param outputFile is the out put file that we write to
 * @param path is the path of the file
 * @param oneStudent is the current dtudent to handle.
 */
void compile(char *cFile, char *inputFile, char *outputFile, char *path, Student *oneStudent) {
    int in, out;
    char *arguments[3];
    char upDirectory[STUDENTS];
    arguments[0] = "gcc";
    arguments[1] = cFile;
    arguments[2] = NULL;
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        int result = execvp(arguments[0], arguments);

    } else {// fathers process
        int status;
        if (waitpid(pid, &status, 0) > 0) {
            if (WEXITSTATUS(status) != 0) {


                char *studentName = (char *) malloc(FILENAME_MAX * sizeof(char));
                char *name;
                strcpy(studentName, cFile);
                name = strtok(studentName, "/");

                while ((name = strtok(NULL, "/") ) != NULL) {
                    strcpy(upDirectory, name);
                    name = strtok(NULL, "/");
                }
                strcpy(oneStudent->name, upDirectory);
                oneStudent->grade = 0;
                strcpy(oneStudent->resultCompare, "COMPILATION_ERROR");
                //write(STDERR, ERROR, SIZEERROR);
                free(studentName);
                return;
            }
        }else{
            write(STDERR, ERROR, SIZEERROR);
            exit(FAIL);
        }
    }

    // execute the file we have compiled
    execute(path, inputFile, outputFile, oneStudent);
}

/**
 * this is a recursive funtion that handels each file and each directory. it finds out if there is a c file
 * and than sends it to the compile function and so on or it finds out that there is no c file and continues to the next student.
 * @param directoryName is the path of the current directory.
 * @param inputFile is the input for the program
 * @param outputFile is the out put file that we write to
 * @param path is the path of the file
 * @param oneStudent is the current dtudent to handle.
 * @param i the number of the student in the strudents array
 * @return integer to find out if there was a c file
 */
int
handleDirectory(char directoryName[STUDENTS], char *inputFile, char *outputFile, char *path, Student *student, int i) {
    struct dirent *pDirent;
    DIR *pDir;
    //int i = 0;
    int length = 0;

    char insideDir[STUDENTS], upDirectory[STUDENTS];
    char *pInsideDir = insideDir, *pUpDirectory = upDirectory; //TODO delete
    if ((pDir = opendir(directoryName)) == NULL) // TODO closedir
        exit(1);

    while (directoryName[length] != '\0') {
        length++;
    }

    while ((pDirent = readdir(pDir)) != NULL) {

        if ((directoryName[length - 1] != 'c') && (directoryName[length - 2] != '.') && pDirent->d_type == DT_DIR) {


            if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
                continue;
            }
            strcpy(insideDir, directoryName);
            strcat(insideDir, "/");
            strcat(insideDir, pDirent->d_name);
            int res = handleDirectory(insideDir, inputFile, outputFile, directoryName, student, i);
            if (res)
                return 1;
            //TODO one finding it is a directory - update the details get to the else some how

        } else if (pDirent->d_type == DT_REG) {
            int nameLength = strlen(pDirent->d_name);
            if ((pDirent->d_name[nameLength - 1] == 'c') && (pDirent->d_name[nameLength - 2] == '.')) {
                char *studentName = (char *) malloc(FILENAME_MAX * sizeof(char));
                char *name;
                strcpy(studentName, directoryName);
                name = strtok(studentName, "/");

                while (name != NULL) {
                    strcpy(upDirectory, name);
                    name = strtok(NULL, "/");
                }

                strcpy(student->name, upDirectory);//updating the name of the student
                strcat(directoryName, "/");
                strcat(directoryName, pDirent->d_name);
                compile(directoryName, inputFile, outputFile, path, student);
                free(studentName);
                return 1;
            }
            // it is not a c file and we ended up in a directory
        }
    }
    closedir(pDir);
    return 0;
}

/**
 * makes the result file containing the info of each student
 * @param students is the students array
 * @param numOfStudents is the number of students.
 */
void makeResultCSVFile(Student **students, int numOfStudents) {
    int index;
    int fdCsv = open("results.csv", O_WRONLY | O_TRUNC | O_CREAT , S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (fdCsv == FAIL) {
        write(STDERR, ERROR, SIZEERROR);
        exit(FAIL);
    } else {
        int r = dup2(fdCsv, 1);
        char line[STUDENTS] = {'\0'};
        for (index = 0; index < numOfStudents; index++) {
            //memset(line, '\0', STUDENTS);
            printf("%s,%d,%s\n", students[index]->name, students[index]->grade,
                   students[index]->resultCompare);
        }

        //write(fdCsv, line, STUDENTS);
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
        makeConfigurationFile(info, argv[1]);

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
            int wasCFile = handleDirectory(tempDir, info[1], info[2], path, students[i], i);
            if (!wasCFile) {
                strcpy(students[i]->name, pDirent->d_name); //updating the name of the student
                students[i]->grade = 0;
                strcpy(students[i]->resultCompare, "NO_C_FILE");
            }
            i++;
        }

        makeResultCSVFile(students, i);

        for (j = 0; j < i; j++) {
            free(students[j]);
        }
        closedir(pDir);
        return 0;
    }
}


