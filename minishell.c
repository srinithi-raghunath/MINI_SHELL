 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>

#define MAX_LINE 80 // Maximum length of command
#define MAX_ARGS (MAX_LINE / 2 + 1) // Maximum number of arguments

pthread_mutex_t lock; // Mutex for thread synchronization

// Color codes
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"

// Function prototypes
void parse_input(char *input, char **args);
void execute_command(char **args);
void create_file(char *filename);
void create_directory(char *dirname);
void write_to_file(char *filename);
void perform_arithmetic(char **args);
void create_file_in_directory(char *dirname, char *filename);
void open_file(char *filename);
void compile_and_run(char *filename);
void search_file(char *filename, char *search_term);
void display_help();
void redirect_io(char **args);
void change_permissions(char *permissions, char *filename);
int check_permissions(const char *filename, const char *mode);

int main(void) {
    char input[MAX_LINE]; // Command line input
    char *args[MAX_ARGS]; // Command line arguments
    int should_run = 1; // Flag to determine when to exit program

    pthread_mutex_init(&lock, NULL); // Initialize mutex

    // Display the prompt only at the start
    printf("\n================================================================================================================\n");
    printf("                                        Mini Shell (mini_sh)    \n");
    printf("================================================================================================================\n");
    printf("mini_sh> ");
    fflush(stdout);

    while (should_run) {
        // Read user input
        if (fgets(input, MAX_LINE, stdin) == NULL) {
            break; // Exit on EOF
        }

        // Check for the exit command
        if (strncmp(input, "exit", 4) == 0) {
            should_run = 0; // Set flag to exit
            continue; // Skip the rest of the loop
        }

        parse_input(input, args);
        execute_command(args);

        // Display the prompt again after executing the command
        printf("mini_sh> ");
        fflush(stdout);
    }

    pthread_mutex_destroy(&lock); // Clean up mutex
    printf("Thanks for using mini_sh! Goodbye!\n"); // Exit message
    return 0;
}

// Function to parse user input into command and arguments
void parse_input(char *input, char **args) {
    char *token;
    int index = 0;

    // Tokenize the input string
    token = strtok(input, "\n"); // Read until newline
    char *command = strtok(token, " "); // Separate command from content
    args[index++] = command;

    // Get additional arguments
    char *arg = strtok(NULL, " "); // Get the first argument
    while (arg) {
        args[index++] = arg; // Add the argument
        arg = strtok(NULL, " ");
    }

    args[index] = NULL; // Null-terminate the args array
}

// Function to execute a command
void execute_command(char **args) {
    if (args[0] == NULL) return; // If no command, return

    if (strcmp(args[0], "help") == 0) {
        display_help(); // Show help information
    } else if (strcmp(args[0], "createfile") == 0 && args[1] != NULL) {
        create_file(args[1]); // Create file
    } else if (strcmp(args[0], "createdir") == 0 && args[1] != NULL) {
        create_directory(args[1]);
    } else if (strcmp(args[0], "createfilein") == 0 && args[1] != NULL && args[2] != NULL) {
        create_file_in_directory(args[1], args[2]); // Create file in directory
    } else if (strcmp(args[0], "openfile") == 0 && args[1] != NULL) {
        open_file(args[1]); // Open and read file
    } else if (strcmp(args[0], "compile") == 0 && args[1] != NULL) {
        compile_and_run(args[1]); // Compile and run the C file
    } else if (strcmp(args[0], "search") == 0 && args[1] != NULL && args[2] != NULL) {
        search_file(args[1], args[2]); // Search in the file
    } else if (strcmp(args[0], "chmod") == 0 && args[1] != NULL && args[2] != NULL) {
        change_permissions(args[1], args[2]); // Change file permissions
    } else {
        redirect_io(args); // Handle input/output redirection
 }
}

// Function to display help information
void display_help() {
    printf("\n+-------------------------------------------------------------------------------------------------------+\n");
    printf("|                                     %sHelp Menu%s                                                        |\n", COLOR_GREEN, COLOR_RESET);
    printf("+-------------------------------------------------------------------------------------------------------+\n");
    printf("| %s1. createfile <filename>%s            Creates a new file with the specified filename.                  |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s2. createdir <dirname>%s              Creates a new directory with the specified dirname.              |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s3. createfilein <dirname> <filename>%s Creates a new file in the specified directory.                  |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s4. openfile <filename>%s               Opens and displays the contents of the specified file.          |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s5. compile <filename>%s                 Compiles the specified C file and runs the program.            |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s6. search <filename> <term>%s           Searches for the specified term in the given file.             |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s7. chmod <permissions> <filename>%s     Changes permissions of a file.                                 |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s8. help%s                                Displays this help information.                               |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s9. exit%s                                Exits the shell.                                              |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s10. Input/Output Redirection:%s         Use '<' to redirect input and '>' to redirect output.          |\n", COLOR_CYAN, COLOR_RESET);
    printf("| %s11. Background Execution:%s              Use '&' at the end of a command to run it in the background.  |\n", COLOR_CYAN, COLOR_RESET);
    printf("+-------------------------------------------------------------------------------------------------------+\n");
}

// Function to create a file
void create_file(char *filename) {
    pthread_mutex_lock(&lock); // Lock for thread safety
    FILE *file = fopen(filename, "w");
    if (file) {
        printf("File created: %s\n", filename);
        write_to_file(filename); // Open the file for writing commands
    } else {
        perror("Failed to create file");
    }
    pthread_mutex_unlock(&lock); // Unlock
}

// Function to create a directory
void create_directory(char *dirname) {
    pthread_mutex_lock(&lock); // Lock for thread safety
    if (mkdir(dirname, 0755) == 0) {
        printf("Directory created: %s\n", dirname);
    } else {
        perror("Failed to create directory");
    }
    pthread_mutex_unlock(&lock); // Unlock
}

// Function to create a file inside a specified directory
void create_file_in_directory(char *dirname, char *filename) {
    char filepath[MAX_LINE];

    // Construct the full path for the new file
    snprintf(filepath, sizeof(filepath), "%s/%s", dirname, filename);
    create_file(filepath); // Use the create_file function to create the file
}

// Function to interactively write to a file
void write_to_file(char *filename) {
    char line[MAX_LINE];
    FILE *file = fopen(filename, "a"); // Open file in append mode

    printf("Type your commands (type 'done' to finish):\n");

    while (1) {
        printf("> "); // Prompt for command
        fflush(stdout);

        if (fgets(line, MAX_LINE, stdin) == NULL) {
            break; // Exit on EOF
        }

        // Check for the 'done' command to finish writing
        if (strncmp(line, "done", 4) == 0) {
            break;
        }

        // Write the command to the file
        fprintf(file, "%s", line);
    }

    fclose(file);
 printf("Finished writing to %s\n", filename);
}

// Function to compile and run a C file
void compile_and_run(char *filename) {
    pthread_mutex_lock(&lock); // Lock for thread safety
    char command[MAX_LINE];

    // Compile the C file
    snprintf(command, sizeof(command), "gcc %s -o temp_program", filename);
    int compile_status = system(command);

    if (compile_status == 0) {
        printf("Compilation successful. Running the program...\n");

        // Use fork and exec to run the compiled program
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execl("./temp_program", "./temp_program", (char *)NULL);
            perror("Execution failed"); // If exec fails
            exit(1);
        } else if (pid > 0) {
            // Parent process
            wait(NULL); // Wait for the child process to finish
        } else {
            perror("Fork failed");
        }
    } else {
        printf("Compilation failed.\n");
    }

    pthread_mutex_unlock(&lock); // Unlock
}

// Function to open and read a file
void open_file(char *filename) {
    if (check_permissions(filename, "r") == 0) {
        printf("Insufficient permissions to read %s.\n", filename);
        return;
  }

    pthread_mutex_lock(&lock); // Lock for thread safety
    FILE *file = fopen(filename, "r"); // Open file in read mode

    if (file) {
        char line[MAX_LINE];
        printf("Contents of %s:\n", filename);
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line); // Print each line
        }
        fclose(file);
    } else {
        perror("Failed to open file");
    }
    pthread_mutex_unlock(&lock); // Unlock
}

// Function to search for a term in a file
void search_file(char *filename, char *search_term) {
    if (check_permissions(filename, "r") == 0) {
        printf("Insufficient permissions to read %s.\n", filename);
        return;
    }

    pthread_mutex_lock(&lock); // Lock for thread safety
    FILE *file = fopen(filename, "r"); // Open file in read mode

    if (file) {
        char line[MAX_LINE];
        int line_number = 0;
        int found = 0;

        while (fgets(line, sizeof(line), file)) {
            line_number++;
            if (strstr(line, search_term) != NULL) {
                printf("Line %d: %s", line_number, line); // Print matching line
                found = 1;
            }
        }
        if (!found) {
            printf("No matches found for '%s' in %s.\n", search_term, filename);
        }

        fclose(file);
    } else {
        perror("Failed to open file");
    }
    pthread_mutex_unlock(&lock); // Unlock
}

// Function to redirect input/output
void redirect_io(char **args) {
    int i = 0;
    int background = 0;
    char *input_file = NULL;
    char *output_file = NULL;
    int input_redirected = 0, output_redirected = 0;

    // Check for redirection symbols
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0) {
            input_file = args[i + 1];
            input_redirected = 1;
            args[i] = NULL; // Nullify the symbol
        } else if (strcmp(args[i], ">") == 0) {
            output_file = args[i + 1];
            output_redirected = 1;
            args[i] = NULL; // Nullify the symbol
        } else if (strcmp(args[i], "&") == 0) {
            background = 1;
            args[i] = NULL; // Nullify the symbol
        }
        i++;
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (input_redirected) {
            if (check_permissions(input_file, "r") == 0) {
  perror("Insufficient permissions for input file");
                exit(1);
            }
            int in_fd = open(input_file, O_RDONLY);
            if (in_fd < 0) {
                perror("Failed to open input file");
                exit(1);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (output_redirected) {
            int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("Failed to open output file");
                exit(1);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        execvp(args[0], args);
        perror("Execution failed");
        exit(1);
    } else if (pid > 0) {
        if (!background) {
            wait(NULL); // Wait for child to finish
        }
    } else {
        perror("Fork failed");
    }
}

// Function to change file permissions
void change_permissions(char *permissions, char *filename) {
    int mode = 0;

    // Parse the permission string
    for (int i = 0; permissions[i] != '\0'; i++) {
 if (permissions[i] == 'r') mode |= S_IRUSR | S_IRGRP | S_IROTH;
        if (permissions[i] == 'w') mode |= S_IWUSR | S_IWGRP | S_IWOTH;
        if (permissions[i] == 'x') mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    }

    // Change the file permissions
    if (chmod(filename, mode) == 0) {
        printf("Permissions changed for %s\n", filename);
    } else {
        perror("Failed to change permissions");
    }
}

// Function to check permissions before accessing a file
int check_permissions(const char *filename, const char *mode) {
    struct stat file_stat;
    if (stat(filename, &file_stat) < 0) {
        perror("stat");
        return -1; // Error in checking permissions
    }

    // Check for read permission
    if (mode[0] == 'r' && !(file_stat.st_mode & S_IRUSR)) {
        return 0; // Denied
    }

    // Check for write permission
    if (mode[1] == 'w' && !(file_stat.st_mode & S_IWUSR)) {
        return 0; // Denied
    }

    // Check for execute permission
    if (mode[2] == 'x' && !(file_stat.st_mode & S_IXUSR)) {
        return 0; // Denied
    }

    return 1; // Granted
}

// Function to perform arithmetic operations
void perform_arithmetic(char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        printf("Insufficient arguments for arithmetic operation.\n");
        return;
    }

    double num1 = atof(args[1]);
    double num2 = atof(args[2]);
    double result = 0;

    if (strcmp(args[0], "add") == 0) {
        result = num1 + num2;
        printf("Result: %.2f\n", result);
    } else if (strcmp(args[0], "sub") == 0) {
        result = num1 - num2;
        printf("Result: %.2f\n", result);
    } else if (strcmp(args[0], "mul") == 0) {
        result = num1 * num2;
        printf("Result: %.2f\n", result);
    } else if (strcmp(args[0], "div") == 0) {
        if (num2 == 0) {
            printf("Error: Division by zero.\n");
            return;
        }
        result = num1 / num2;
        printf("Result: %.2f\n", result);
    } else if (strcmp(args[0], "mod") == 0) {
        if ((int)num2 == 0) {
            printf("Error: Division by zero.\n");
            return;
        }
        result = (int)num1 % (int)num2; // Modulus operation
        printf("Result: %.2f\n", result);
    } else {
        printf("Unknown operation: %s\n", args[0]);
    }
}
