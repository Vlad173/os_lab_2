/* Епанешников В.С. М80-206Б-19
Родительский процесс создаёт дочерний процесс. Первой строкой пользователь пишет в консоль имя файла, которое будет передано
при создании дочернего процесса. Родительский процесс передает команды пользователя через pipe1, который связан с стандартным 
входным потоком дочернего процесса. Результаты своей работы дочерний процесс пишет в созданный им файл.

Вариант 1
Пользователь вводит команды вида: «число число число<endline>». Далее эти числа передаются от родительского процесса в дочерний.
Дочерний процесс считает их сумму и выводит её в файл. Числа имеют тип int.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
 #include <sys/wait.h>
#include <ctype.h>
#include <stdbool.h>

#define STDIN 0
#define STDOUT 1
#define MIN_CAP 4

char* read_string(int fd) {
    size_t str_size = 0;
    char* str = (char*) malloc(sizeof(char) * MIN_CAP);
    if (str == NULL) {
        perror("Malloc error");
        exit(-1);
    }
    char c;
    size_t cap = MIN_CAP;
    while (read(fd, &c, sizeof(char)) == 1) {
        if (c == '\n') {
            break;
        }
        str[(str_size)++] = c;
        if (str_size == MIN_CAP) {
            str = (char*) realloc(str, sizeof(char) * cap * 3 / 2);
            if (str == NULL) {
                perror("Realloc error");
                exit(-2);
            }
        }
    }
    str[str_size] = '\0';
    return str;
}

int str_length(char* str) {
    int length = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        ++length;
    }
    return length;
}

char* int_to_string(sum) {
    int size = 0;
    int temp_sum = sum;
    while (temp_sum != 0) {
        temp_sum /= 10;
        ++size;
    }

    bool is_positive = true;
    if (sum < 0) {
        is_positive = false;
        sum *= -1;
        size++;
    }

    char* str = (char*) malloc(sizeof(char) * (size + 1));
    str[size] = '\0';
    str[0] = 'k';
    str[1] = 'k';
    printf("%s\n", str);
    for (int i = size - 1; i >= 0; --i) {
        if (!is_positive && i == 0) {
            str[0] = '-';
        } else {
            str[i] = sum % 10 + '0';
            sum /= 10;
        }
        printf("%s\n", str);
    }

    return str;
}

void child_work(char str[], char path[]) {
    mode_t mode = S_IRUSR | S_IWUSR;
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
    int file = open(path, flags, mode);
    if (file == -1) {
        perror("Open error");
         exit(7);
    }  

    int sum = 0, number = 0, is_positive = 1;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        if (str[i] == '-') {
            is_positive = -1;
            continue;
        }
        if (isdigit(str[i])) {
            number = number * 10 + str[i] - '0';
        }
        else if (str[i] == ' ') {
            sum += number * is_positive;
            number = 0;
            is_positive = 1;
        }
        
    }
    sum += number * is_positive;
    char* answer = int_to_string(sum);
    int answer_size = str_length(answer);

    write(file, answer, sizeof(char) * answer_size);
    close(file); 
    free(answer);
}

int main() {
    int fd[2], child_status;
    if (pipe(fd) < 0){
        perror("Can\'t create pipe");
        exit(-3); 
    } 

    int id = fork();
    
    if (id == -1) { 
        perror("Can\'t fork child");
        exit(-4);
    } else if (id > 0) {
        close(fd[0]);

        char* path = read_string(STDIN);
        size_t path_size = str_length(path);
        if (write(fd[1], &path_size, sizeof(size_t)) != sizeof(size_t)) {
            perror("Write error");
            exit(1);
        }
        if (write(fd[1], path, sizeof(char) * path_size) != path_size) {
            perror("write error");
            exit(2);
        }

        char* str = read_string(STDIN);
        size_t str_size = str_length(str);
        if (write(fd[1], &str_size, sizeof(size_t)) != sizeof(size_t)) {
            perror("Write error");
            exit(3);
        }
        if (write(fd[1], str, sizeof(char) * str_size) != str_size) {
            perror("write error");
            exit(4);
        }

        if (wait(&child_status) == -1) {
            perror("wait() error");
            exit(5);
        }

        if (WIFEXITED(child_status)) {
            printf("The child process exited normally, with exit code %d\n", WEXITSTATUS(child_status));
        } else {
            printf("The child process exited abnormally\n");
        }

        free(str);
        close(fd[1]);
    }
    else {
        close(fd[1]);

        size_t path_size;
        if (read(fd[0], &path_size, sizeof(size_t)) != sizeof(size_t)) {
            perror("Read error");
            exit(1);
        }
        char* path = (char*) malloc(sizeof(char) * path_size);
        if (path == NULL) {
            perror("Malloc error");
            exit(2);
        }
        if (read(fd[0], path, sizeof(char) * path_size) != path_size) {
            perror("Read error");
            exit(3);
        }

        size_t str_size;
        if (read(fd[0], &str_size, sizeof(size_t)) != sizeof(size_t)) {
            perror("Read error");
            exit(4);
        }
        char* str = (char*) malloc(sizeof(char) * str_size);
        if (str == NULL) {
            perror("Malloc error");
            exit(5);
        }
        if (read(fd[0], str, sizeof(char) * str_size) != str_size) {
            perror("Read error");
            exit(6);
        }
        
        close(fd[0]);
    
        child_work(str, path);
        
        free(str);
        free(path);
        printf("Child exit\n");
    }
    
    return 0;
}