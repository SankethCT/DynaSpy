#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>

volatile sig_atomic_t running = 1; 

#define MAX_LIBRARIES 100
#define MAX_ALERTED_LIBRARIES 100

char *alert_libraries[MAX_LIBRARIES];
int alert_library_count = 0;

char *alerted_libraries[MAX_ALERTED_LIBRARIES];
int alerted_library_count = 0;

// Securely erase a memory buffer
void secure_memset(void *v, int c, size_t n) {
#ifdef __STDC_LIB_EXT1__
    memset_s(v, n, c, n);
#else
    volatile unsigned char *p = v;
    while (n--) *p++ = c;
#endif
}

// Validate the output filename (for demonstration purposes only)
int validate_output_filename(const char *filename) {
    const char *invalid_chars = "/\\";
    return strcspn(filename, invalid_chars) == strlen(filename);
}

void add_alert_library(char *library) {
  if (alert_library_count < MAX_LIBRARIES) {
    alert_libraries[alert_library_count++] = library;
  }
}

int check_alert_library(char *library) {
  char *base_library = basename(library);
  for (int i = 0; i < alert_library_count; i++) {
    if (strcmp(alert_libraries[i], base_library) == 0) {
      return 1;
    }
  }
  return 0;
}

int check_alerted_library(char *library) {
  for (int i = 0; i < alerted_library_count; i++) {
    if (strcmp(alerted_libraries[i], library) == 0) {
      return 1;
    }
  }
  return 0;
}

void add_alerted_library(char *library) {
  if (alerted_library_count < MAX_ALERTED_LIBRARIES) {
    alerted_libraries[alerted_library_count++] = library;
  }
}

int monitor_libraries(char *application, FILE *output) {
  int app_pid;
  char cmd[100];
  char path[100];
  FILE *cmd_output;

  sprintf(cmd, "pidof %s", application);
  cmd_output = popen(cmd, "r");
  if (cmd_output == NULL) {
    perror("popen");
    return 1;
  }

  if (fscanf(cmd_output, "%d", &app_pid) == EOF) {
    pclose(cmd_output);
    return 1;
  }
  pclose(cmd_output);

  sprintf(path, "/proc/%d/maps", app_pid);

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  char buffer[1024];
  int bytes_read;
  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    char *line = buffer;
    for (int i = 0; i < bytes_read; i++) {
      if (buffer[i] == '\n') {
        buffer[i] = '\0';
        char *library = strstr(line, "/");
        if (library) {
          fprintf(output, "Library loaded by %s: %s\n", application, library);
          if (check_alert_library(library) && !check_alerted_library(library)) {
            printf("ALERT: Library %s has been loaded by %s.\n", library, application);
            add_alerted_library(library);
          }
        }
        line = buffer + i + 1;
      }
    }
  }

  close(fd);
  return 0;
}

void signal_handler(int signum) {
  if (signum == SIGINT) {
    running = 0;
  }
}

void print_help(char *prog_name) {
  printf("Usage: %s [OPTIONS] <application> [library1] [library2] ...\n", prog_name);
  printf("\n");
  printf("Options:\n");
  printf("  -o <file>     Specify the output file for the list of loaded libraries (default: loaded_libraries.txt)\n");
  printf("  --help        Show this help message and exit.\n");
  printf("\n");
  printf("Arguments:\n");
  printf("  <application> The name of the application you want to monitor.\n");
  printf("  [library1]    Optional list of libraries to receive alerts when loaded by the application.\n");
}


char *get_argument(int *index, int argc, char *argv[]) {
  if (*index + 1 < argc) {
    (*index)++;
    return argv[*index];
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_help(argv[0]);
    return 1;
  }

  int app_arg_index = -1;
  char *output_filename = "loaded_libraries.txt";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "-o") == 0) {
      output_filename = get_argument(&i, argc, argv);
      if (!output_filename) {
        printf("Error: -o requires an output file argument.\n");
        return 1;
      }
    } else {
      if (app_arg_index == -1) {
        app_arg_index = i;
      } else {
        add_alert_library(argv[i]);
      }
    }
  }

  if (app_arg_index == -1) {
    printf("Error: Application argument is missing.\n");
    print_help(argv[0]);
    return 1;
  }

  
    // Validate output filename
    if (!validate_output_filename(output_filename)) {
        printf("Error: Invalid output filename.\n");
        return 1;
    }

    // Set umask before opening the file
    umask(S_IWGRP | S_IWOTH);

    FILE *output = fopen(output_filename, "w");
    if (!output) {
        perror("Error: Unable to open output file");
        return 1;
    }

    signal(SIGINT, signal_handler);

    while (running) {
        monitor_libraries(argv[app_arg_index], output);
        sleep(1);
    }

    printf("Exiting gracefully...\n");
    fclose(output);

    // Securely erase sensitive data from memory
    secure_memset(alert_libraries, 0, sizeof(alert_libraries));
    secure_memset(alerted_libraries, 0, sizeof(alerted_libraries));

    return 0;
}
