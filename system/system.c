#include <system.h>

#include <stdio.h>
#include <math.h>

// Output file name for saving the system state
static const char *OUTPUT_FILE = "input.csv";

static const char *TABLE_OUTLINE =
    "+--------+-------+---------------+---------------+----------------+--------------+------------+----------------+\n";

/**
 * Create a random uniform variable between the `start` and `stop` ranges.
 *
 * This also rounds it to 2 decimal places.
 */
static float UniformVar(float start, float stop)
{
    float var = start + ((float)rand() / RAND_MAX) * (stop - start);
    var = (float)round(var * 100.f) / 100.f;

    return var;
}

/**
 * Internally setup the following system columns:
 * - `arrival_times`
 *
 * NOTE: This should be called immediately after initializing the IAT and service times columns.
 */
static void SetupSystemColumns(System *system)
{
    system->arrival_times = calloc(system->no_customers, sizeof(float));
    system->service_start_times = calloc(system->no_customers, sizeof(float));
    system->service_end_times = calloc(system->no_customers, sizeof(float));

    system->wait_times = calloc(system->no_customers, sizeof(float));
    system->times_in_system = calloc(system->no_customers, sizeof(float));

    printf(TABLE_OUTLINE);

    printf("| %-6s | %-5s | %-13s | %-13s | %-14s | %-12s | %-10s | %-14s |\n",
           "No.", "IAT", "Service Times", "Arrival Times", "Service Starts", "Service Ends", "Wait Times", "Time in System");

    printf(TABLE_OUTLINE);

    system->arrival_times[0] = system->iats[0];
    system->service_start_times[0] = system->iats[0];
    system->service_end_times[0] = system->service_start_times[0] + system->service_times[0];
    system->wait_times[0] = system->service_start_times[0] - system->arrival_times[0];
    system->times_in_system[0] = system->service_end_times[0] - system->arrival_times[0];

    float total_iat = system->iats[0];
    float total_service_time = system->service_times[0];
    float total_wait_time = system->wait_times[0];
    float total_time_in_system = system->times_in_system[0];

    printf("| %6d | %5.2f | %13.2f | %13.2f | %14.2f | %12.2f | %10.2f | %14.2f |\n",
           0, system->iats[0], system->service_times[0],
           system->arrival_times[0], system->service_start_times[0], system->service_end_times[0],
           system->wait_times[0], system->times_in_system[0]);

    for (size_t i = 1; i < system->no_customers; i++)
    {
        system->arrival_times[i] = system->arrival_times[i - 1] + system->iats[i];

        if (system->arrival_times[i] < system->service_end_times[i - 1])
        {
            system->service_start_times[i] = system->service_end_times[i - 1];
        }
        else
        {
            system->service_start_times[i] = system->arrival_times[i];
        }

        system->service_end_times[i] = system->service_start_times[i] + system->service_times[i];
        system->wait_times[i] = system->service_start_times[i] - system->arrival_times[i];
        system->times_in_system[i] = system->service_end_times[i] - system->arrival_times[i];

        total_iat += system->iats[i];
        total_service_time += system->service_times[i];
        total_wait_time += system->wait_times[i];
        total_time_in_system += system->times_in_system[i];

        printf("| %6d | %5.2f | %13.2f | %13.2f | %14.2f | %12.2f | %10.2f | %14.2f |\n",
               i, system->iats[i], system->service_times[i],
               system->arrival_times[i], system->service_start_times[i], system->service_end_times[i],
               system->wait_times[i], system->times_in_system[i]);
    }

    printf(TABLE_OUTLINE);

    printf("| TOTALS | %4.2f | %13.2f | %13s | %14s | %12s | %10.2f | %14.2f |\n",
           total_iat, total_service_time, "", "", "", total_wait_time, total_time_in_system);

    printf(TABLE_OUTLINE);
}

System *InitializeSystem(size_t n)
{
    // Allocate memory for the system struct and initialize the number of customers
    System *system = malloc(sizeof(System));
    system->no_customers = n;

    // Allocate memory for the IAT and sevrice time arrays

    float *iats = calloc(n, sizeof(float));
    float *service_times = calloc(n, sizeof(float));

    // This maps the allocated arrays to the system struct
    system->iats = iats;
    system->service_times = service_times;

    // Create the input file
    FILE *file = fopen(OUTPUT_FILE, "w");

    fprintf(file, "IAT,Service Time\n");

    // Generate the IAT and service time for each customer, storing it in the arrays
    for (size_t i = 0; i < n; i++)
    {
        iats[i] = UniformVar(1, 8);
        service_times[i] = UniformVar(1, 5);
        fprintf(file, "%.2f,%.2f\n", iats[i], service_times[i]);
    }

    // Close the file to save it
    fclose(file);

    // Run the simulation
    SetupSystemColumns(system);

    return system;
}

System *InitializeSystemFromFile(const char *input_file)
{
    // ALlocate memory for the system struct
    System *system = malloc(sizeof(System));

    // Open the input file containing the IATs and service times
    FILE *file = fopen(input_file, "r");

    // Handle any errors
    if (file == NULL)
    {
        perror("Could not open file");
        free(system);
        return NULL;
    }

    // Count the number of lines in the file, this will indicate how many customers there are
    char buffer[SYSTEM_BUFFER_LEN];
    size_t n = 0;
    while (fgets(buffer, SYSTEM_BUFFER_LEN, file))
    {
        n++;
    }

    n--;  // Exclude the column headers of the CSV file
    system->no_customers = n;

    fseek(file, SEEK_SET, 0); // Go back to the beginning of the file

    fgets(buffer, SYSTEM_BUFFER_LEN, file); // Skip the first line

    // Now read each line into the arrays needed for the system
    float *iats = calloc(n, sizeof(float));
    float *service_times = calloc(n, sizeof(float));

    // Map the allocated arrays to the system struct
    system->iats = iats;
    system->service_times = service_times;
    size_t i = 0;

    // Read each line of the file into a buffer
    while (fgets(buffer, SYSTEM_BUFFER_LEN, file))
    {
        float iat, service_time;
        sscanf(buffer, "%f,%f", &iat, &service_time);  // This copies the numbers in the buffer string into the iat and service time variables
        iats[i] = iat;
        service_times[i] = service_time;
        i++;
    }

    // Close the file once done
    fclose(file);

    // Run the simulation
    SetupSystemColumns(system);

    return system;
}

void DestroySystem(System *system)
{
    free(system->iats);
    free(system->service_times);

    free(system->arrival_times);
    free(system->service_start_times);
    free(system->service_end_times);

    free(system->wait_times);
    free(system->times_in_system);

    free(system);
}