
# DynaSpy

The DynaSpy is a Linux-based utility designed to monitor and log libraries loaded by a specified application. It also alerts the user when specific libraries, defined by the user, are loaded by the application.

## Features

- Monitor libraries loaded by a specified application
- Alert the user when specified libraries are loaded
- Write library load events to an output file
- Provides a user-friendly command-line interface
- Securely handles sensitive data
- Demonstrates dropping privileges if run as root (for demonstration purposes)

## Compilation

To compile the DynaSpy, use the following command:

gcc -o dynaspy dynaspy.c -ldl

This command will generate an executable named `dynaspy`.

## Usage

To use the DynaSpy, execute the following command:

./dynaspy [OPTIONS] <application> [library1 [library2 ...]]

- `OPTIONS`:
  - `-o <output_filename>`: Specify the output file to write library load events
  - `--help`: Display help information and usage
- `<application>`: The application you want to monitor
- `[library1 [library2 ...]]`: (Optional) A space-separated list of library names to alert the user when loaded by the application

Example:

./dynaspy -o output.txt my_application libc.so.6 ld-linux-x86-64.so.2


This command will monitor the `my_application` process, log library load events to `output.txt`, and alert the user when either `libc.so.6` or `ld-linux-x86-64.so.2` are loaded.

## User Manual

### Getting Started

1. Compile the DynaSpy as instructed in the "Compilation" section.
2. Run the compiled executable with the appropriate command-line arguments as described in the "Usage" section.

### Monitoring an Application

To monitor an application, you must provide the application name as the first non-option argument. The DynaSpy will then monitor the specified application and log library load events.

### Specifying Libraries to Alert

You can provide a list of library names as additional arguments after the application name. The DynaSpy will alert you when any of these libraries are loaded by the application.

### Writing Library Load Events to an Output File

Use the `-o` option followed by the output file name to write library load events to a file. The tool will create the specified output file with restricted permissions.

### Stopping the Tool

To stop the DynaSpy, press `Ctrl+C`. The tool will exit gracefully, closing the output file and securely erasing sensitive data from memory.

### Help and Usage Information

To display help and usage information, run the DynaSpy with the `--help` option.
