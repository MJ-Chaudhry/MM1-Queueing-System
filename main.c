#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "system.h"

int main(int argc, char **argv)
{
    // Seed the random function
    srand((unsigned int)time(NULL));

    System *system = NULL;

    // Parse the command line arguments
    // The initialize system functions will automatically start the simulation as well.
    switch (argc)
    {
    case 2:
        // Generates N uniform arrival intervals and service times and initializes the system

        size_t n = atoi(argv[1]);
        if (n == 0)
        {
            printf("N must be a positive non-zero integer!\n");
            return -1;
        }
        system = InitializeSystem(n);
        break;
    case 3:
        // Initializes the system based on an input file with arrival intervals and service times
        if (strcmp("-f", argv[1]) != 0)
        {
            printf("Invalid flag passed, must be -f\n");
            return -1;
        }
        system = InitializeSystemFromFile(argv[2]);
        if (system == NULL)
        {
            return -1;
        }
        break;
    default:
        printf("Invalid arguments passed! Run the program using:\n");
        printf("./main [N] - initialize both the input file and system\n");
        printf("./main -f [INPUT_FILE] - initialize the system from an existing input file\n");
        return -1;
    }

    DestroySystem(system);

    return 0;
}
