//The main purpose of this code is to understand how the signal can be decoded. 
#include <stdio.h> // For printf (printing output) and scanf (getting input)

// Global array to store the 10 hexadecimal bytes of the command
unsigned char signal[10];

// Function to print the generated signal array in the required format
void print_signal() {
    printf("\nGenerated Signal Code:\n");
    printf("signal[10] = {");
    for (int i = 0; i < 9; ++i) {
        // Print each byte as a 2-digit hexadecimal number (e.g., 0x0A, 0x3B)
        // %02X means: 0 for padding with zero if needed, 2 for width of two, X for uppercase hex
        printf("0x%02X, ", signal[i]);
    }
    printf("0x%02X}\n", signal[9]); // Last byte without a comma after it
}

// Function to determine and set the 'signal' array based on user choices
// power_status: 0 for OFF, 1 for ON
// mode_choice: 1 for Cold, 2 for Water (Dry), 3 for Sun (Heat), 4 for Fan
// temp_value: Desired temperature (e.g., 16-30 degrees Celsius)
// fan_speed_choice: 0 for Default, 1 for Low, 2 for Medium, 3 for High, 4 for Auto (mainly for Cold mode)
void generate_ac_command(int power_status, int mode_choice, int temp_value, int fan_speed_choice) {

    // --- 1. Handle POWER OFF case ---
    // If power is OFF, the other settings (mode, temp, fan) don't matter.
    if (power_status == 0) {
        signal[0] = 0x33;
        signal[1] = 0x28;
        signal[2] = 0x08; // This byte often indicates the OFF state
        signal[3] = 0x18;
        signal[4] = 0x3B;
        signal[5] = 0x3B;
        signal[6] = 0x3B;
        signal[7] = 0x11;
        signal[8] = 0x20;
        signal[9] = 0xA2; // The final byte for the OFF command
        return; // Signal is set for OFF, so we are done with this function.
    }

    // --- 2. Handle POWER ON case ---
    // If power is ON, we set the common bytes first.
    // These bytes are the same for all ON states according to your data.
    signal[0] = 0x33;
    // signal[1], signal[2], signal[3], and signal[9] will be set below
    // based on the mode, temperature, and fan speed.
    signal[4] = 0x3B;
    signal[5] = 0x3B;
    signal[6] = 0x3B;
    signal[7] = 0x11;
    signal[8] = 0x20;

    int current_temp = temp_value; // Use a local variable for temperature

    // --- 2a. MODE selection ---
    // We use a 'switch' statement to handle different modes.
    switch (mode_choice) {
        case 1: // MODE: Cold
            // For Cold mode, temperature should be between 16 and 30.
            // If it's outside this range, we'll silently clamp it.
            if (current_temp < 16) {
                current_temp = 16; // Too low, set to 16
            } else if (current_temp > 30) {
                current_temp = 30; // Too high, set to 30
            }
            // The 4th byte (signal[3]) in the command is the temperature.
            signal[3] = (unsigned char)current_temp;

            // Fan speed settings are specific to Cold mode.
            switch (fan_speed_choice) {
                case 1: // Low Fan Speed
                    signal[1] = 0x88; signal[2] = 0x80; signal[9] = (unsigned char)(0xE2 - current_temp);
                    break;
                case 2: // Medium Fan Speed
                    signal[1] = 0x48; signal[2] = 0x80; signal[9] = (unsigned char)(0x22 - current_temp);
                    break;
                case 3: // High Fan Speed
                    signal[1] = 0x28; signal[2] = 0x80; signal[9] = (unsigned char)(0x42 - current_temp);
                    break;
                case 4: // Auto Fan Speed
                    signal[1] = 0xE8; signal[2] = 0x80; signal[9] = (unsigned char)(0x82 - current_temp);
                    break;
                case 0: // Default fan for Cold mode
                default: // Also handles any unexpected fan_speed_choice for cold mode
                    // This is the original "cold" mode setting from your initial data (e.g., Temp 24)
                    signal[1] = 0x28; // 2nd byte for default cold
                    signal[2] = 0x88; // 3rd byte for default cold
                    signal[9] = (unsigned char)(0x3A - current_temp); // 10th byte calculation
                    break;
            }
            break;

        case 2: // MODE: Water (or Dry mode)
            // For Water mode, temperature and fan speed are usually fixed or ignored.
            signal[1] = 0x84;
            signal[2] = 0x88;
            signal[3] = 0x18; // Fixed value for this mode
            signal[9] = 0xC6; // Fixed value for this mode
            break;

        case 3: // MODE: Sun (or Heat mode)
            // For Sun mode, fan speed is usually fixed or ignored.
            // Temperature should be between 16 and 30.
            if (current_temp < 16) {
                current_temp = 16;
            } else if (current_temp > 30) {
                current_temp = 30;
            }
            signal[1] = 0x22;
            signal[2] = 0x88;
            signal[3] = (unsigned char)current_temp; // 4th byte is the temperature
            signal[9] = (unsigned char)(0x40 - current_temp); // 10th byte calculation
            break;

        case 4: // MODE: Fan only
            // For Fan only mode, temperature and specific fan speeds (low/med/high) are often ignored.
            signal[1] = 0x41;
            signal[2] = 0x88;
            signal[3] = 0x10; // Fixed value for this mode
            signal[9] = 0x11; // Fixed value for this mode
            break;

        default: // Invalid mode choice
            printf("Error: Invalid MODE selected. Turning device OFF as a safety measure.\n");
            generate_ac_command(0, 0, 0, 0); // Default to POWER OFF
            break;
    }
}


// The main part of the program where execution begins
int main() {
    int power_input;
    int mode_input = 1;      // Default to Cold mode if power is on
    int temp_input = 24;     // A common default temperature (e.g., 24 degrees C)
    int fan_speed_input = 0; // 0 means 'default' fan behavior for the selected mode

    printf("--- AC Control Signal Generator (Simple Version) ---\n");

    // 1. Get POWER status from the user
    printf("Enter POWER status (0 for OFF, 1 for ON): ");
    // scanf reads the integer. We check if it successfully read 1 item.
    if (scanf("%d", &power_input) != 1 || (power_input != 0 && power_input != 1)) {
        printf("Invalid POWER input. Assuming OFF.\n");
        power_input = 0; // Default to OFF if input is not 0 or 1
    }
    // This loop clears any leftover characters (like the Enter key) from the input buffer.
    // It's good practice after scanf, especially before another scanf.
    while(getchar() != '\n');


    // Only ask for mode, temp, and fan speed if power is ON
    if (power_input == 1) {
        // 2. Get MODE from the user
        printf("\nEnter MODE:\n");
        printf("  1: Cold\n");
        printf("  2: Water (Dry)\n");
        printf("  3: Sun (Heat)\n");
        printf("  4: Fan\n");
        printf("Choice (1-4, default 1 for Cold): ");
        if (scanf("%d", &mode_input) != 1 || mode_input < 1 || mode_input > 4) {
            printf("Invalid MODE input. Assuming COLD mode (1).\n");
            mode_input = 1; // Default to Cold mode
        }
        while(getchar() != '\n');

        // 3. Get TEMPERATURE (only relevant for Cold and Sun modes)
        if (mode_input == 1 || mode_input == 3) { // If mode is Cold or Sun
            printf("Enter TEMPERATURE (16-30 C, default %d C): ", temp_input);
            if (scanf("%d", &temp_input) != 1) {
                printf("Invalid TEMP input. Using default %d C.\n", temp_input);
                // temp_input keeps its pre-set default value (24)
            }
            // The actual clamping (16-30) happens inside generate_ac_command
            while(getchar() != '\n');
        }


        // 4. Get FAN SPEED (this is most relevant for Cold mode)
        if (mode_input == 1) { // Only ask for specific fan speed if mode is Cold
            printf("Enter FAN SPEED for Cold mode:\n");
            printf("  0: Default (uses mode's standard fan setting)\n");
            printf("  1: Low\n");
            printf("  2: Medium\n");
            printf("  3: High\n");
            printf("  4: Auto\n");
            printf("Choice (0-4, default 0): ");
            if (scanf("%d", &fan_speed_input) != 1 || fan_speed_input < 0 || fan_speed_input > 4) {
                printf("Invalid FAN SPEED input. Assuming Default fan (0).\n");
                fan_speed_input = 0; // Default to 'Default fan'
            }
            while(getchar() != '\n');
        } else {
            // For modes other than Cold, we'll just use the 'default' fan speed (0).
            // The generate_ac_command function will know how to handle this.
            fan_speed_input = 0;
        }
    }

    // Now, call the function to actually create the 10-byte signal
    generate_ac_command(power_input, mode_input, temp_input, fan_speed_input);

    // Finally, print the generated hexadecimal signal
    print_signal();

    return 0; // Indicates that the program finished successfully
}
