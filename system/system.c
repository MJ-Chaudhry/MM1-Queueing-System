#include <system.h>

#include <stdio.h>
#include <math.h>

// Output file name for saving the system state
static const char *OUTPUT_FILE = "input.csv";

static const char *TABLE_TOP =
    "┏━━━━━━━━┳━━━━━━━┳━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━┳━━━━━━━━━━━┳━━━━━━━━━━━┓\n";

static const char *TABLE_OUTLINE =
    "┣━━━━━━━━╋━━━━━━━╋━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━━━━━━━╋━━━━━━━━━━━━╋━━━━━━━━━━━╋━━━━━━━━━━━┫\n";

static const char *TABLE_BOTTOM =
    "┗━━━━━━━━┻━━━━━━━┻━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━┻━━━━━━━━━━━┻━━━━━━━━━━━┛\n";

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
static void RunSimulation(System *system)
{
    system->arrival_times = calloc(system->no_customers, sizeof(float));
    system->service_start_times = calloc(system->no_customers, sizeof(float));
    system->service_end_times = calloc(system->no_customers, sizeof(float));

    system->wait_times = calloc(system->no_customers, sizeof(float));
    system->times_in_system = calloc(system->no_customers, sizeof(float));

    system->no_system = calloc(system->no_customers, sizeof(int));
    system->no_queue = calloc(system->no_customers, sizeof(int));
    system->idle_times = calloc(system->no_customers, sizeof(float));

    printf(TABLE_TOP);

    printf("┃ %-6s ┃ %-5s ┃ %-13s ┃ %-13s ┃ %-14s ┃ %-12s ┃ %-10s ┃ %-14s ┃ %-10s ┃ %-9s ┃ %-9s ┃\n",
           "No.", "IAT", "Arrival Times", "Service Times", "Service Starts", "Service Ends", "Wait Times", "Time in System", "No. System", "No. Queue", "Idle Time");

    printf(TABLE_OUTLINE);

    system->arrival_times[0] = system->iats[0];
    system->service_start_times[0] = system->iats[0];
    system->service_end_times[0] = system->service_start_times[0] + system->service_times[0];
    system->wait_times[0] = system->service_start_times[0] - system->arrival_times[0];
    system->times_in_system[0] = system->service_end_times[0] - system->arrival_times[0];
    system->no_system[0] = 1;
    system->no_queue[0] = 0;
    system->idle_times[0] = 0;

    float total_iat = 0; // Ignore the first customer's IAT as it isn't really an IAT
    float total_service_time = system->service_times[0];
    float total_wait_time = system->wait_times[0];
    float total_time_in_system = system->times_in_system[0];
    float total_idle_time = system->idle_times[0];
    int total_customers_waiting = 0;

    printf("┃ %6d ┃ %5.2f ┃ %13.2f ┃ %13.2f ┃ %14.2f ┃ %12.2f ┃ %10.2f ┃ %14.2f ┃ %10d ┃ %9d ┃ %9.2f ┃\n",
           1, system->iats[0], system->arrival_times[0], system->service_times[0],
           system->service_start_times[0], system->service_end_times[0],
           system->wait_times[0], system->times_in_system[0],
           system->no_system[0], system->no_queue[0], system->idle_times[0]);

    for (size_t i = 1; i < system->no_customers; i++)
    {
        // Update the arrival time in the system
        system->arrival_times[i] = system->arrival_times[i - 1] + system->iats[i];

        // Determine the service start time based on the arrival time and previous service time
        if (system->arrival_times[i] < system->service_end_times[i - 1])
        {
            system->service_start_times[i] = system->service_end_times[i - 1];
        }
        else
        {
            system->service_start_times[i] = system->arrival_times[i];
        }

        // Calculate the service end time, wait time, and time in the system
        system->service_end_times[i] = system->service_start_times[i] + system->service_times[i];
        system->wait_times[i] = system->service_start_times[i] - system->arrival_times[i];
        system->times_in_system[i] = system->service_end_times[i] - system->arrival_times[i];

        if (system->wait_times[i] > 0)
        {
            total_customers_waiting++;
        }

        // Determine the number of people in the system
        int no_system = 0;
        for (size_t j = 0; j < i; j++)
        {
            if (system->service_end_times[j] > system->arrival_times[i])
            {
                no_system++;
            }
        }
        no_system++;

        system->no_system[i] = no_system;
        system->no_queue[i] = no_system - 1;

        float idle_time = system->service_start_times[i] - system->service_end_times[i - 1];

        system->idle_times[i] = idle_time;

        // Update aggregate counters for the total IAT, service time, wait time, time in system, and idle time
        total_iat += system->iats[i];
        total_service_time += system->service_times[i];
        total_wait_time += system->wait_times[i];
        total_time_in_system += system->times_in_system[i];
        total_idle_time += idle_time;

        printf("┃ %6d ┃ %5.2f ┃ %13.2f ┃ %13.2f ┃ %14.2f ┃ %12.2f ┃ %10.2f ┃ %14.2f ┃ %10d ┃ %9d ┃ %9.2f ┃\n",
               i + 1, system->iats[i], system->arrival_times[i], system->service_times[i],
               system->service_start_times[i], system->service_end_times[i],
               system->wait_times[i], system->times_in_system[i],
               system->no_system[i], system->no_queue[i], system->idle_times[i]);
    }

    printf(TABLE_OUTLINE);

    printf("┃ TOTALS ┃ %5.2f ┃ %13s ┃ %13.2f ┃ %14s ┃ %12s ┃ %10.2f ┃ %14.2f ┃ %-10s ┃ %-9s ┃ %9.2f ┃\n",
           total_iat, "", total_service_time, "", "", total_wait_time, total_time_in_system, "", "", total_idle_time);

    printf(TABLE_BOTTOM);

    printf("\nQUEUE STATISTICS:\n━━━━━━━━━━━━━━━━━\n");
    printf("Avergae wait time: %.2f\n", total_wait_time / system->no_customers);
    printf("Probability of customer waiting: %.2f\n", (float)total_customers_waiting / (float)system->no_customers);
    printf("Proportion of idle time: %.2f\n", total_idle_time / system->service_end_times[system->no_customers - 1]);
    printf("Probability server is busy: %.2f\n", 1 - total_idle_time / system->service_end_times[system->no_customers - 1]);
    printf("Average service time: %.2f\n", total_service_time / system->no_customers);
    printf("Average IAT: %.2f\n", total_iat / (system->no_customers - 1));
    printf("Average wait time for those who wait: %.2f\n", total_wait_time / total_customers_waiting);
    printf("Avergae time in system: %.2f\n", total_time_in_system / system->no_customers);
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
    RunSimulation(system);

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

    n--; // Exclude the column headers of the CSV file
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
        sscanf(buffer, "%f,%f", &iat, &service_time); // This copies the numbers in the buffer string into the iat and service time variables
        iats[i] = iat;
        service_times[i] = service_time;
        i++;
    }

    // Close the file once done
    fclose(file);

    // Run the simulation
    RunSimulation(system);

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

    free(system->no_system);
    free(system->no_queue);
    free(system->idle_times);

    free(system);
}