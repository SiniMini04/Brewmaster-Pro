#include <iostream>
#include <Serial.h>
#include <unistd.h> // for usleep

int main() {
    // Initialize the serial port
    Serial serialPort("/dev/ttyUSB0"); // Change this to your serial port

    // Check if the serial port is open
    if (!serialPort.isOpen()) {
        std::cerr << "Error: Unable to open serial port." << std::endl;
        return 1;
    }

    std::cout << "Serial port opened successfully. Monitoring input..." << std::endl;

    // Main loop to monitor serial input
    while (true) {
        // Check if data is available to read from the serial port
        if (serialPort.available()) {
            // Read the data from the serial port
            std::string data = serialPort.read();

            // Print the received data to the console
            std::cout << "Received: " << data << std::endl;
        }

        // Add a small delay to avoid high CPU usage
        // Adjust the delay according to your requirements
        usleep(1000000); // Sleep for 1 second
    }

    return 0;
}
