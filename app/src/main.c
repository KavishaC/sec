/** 22T3 COMP3601 Design Project A
 * File name: main.c
 * Description: Example main file for using the audio_i2s driver for your Zynq audio driver.
 *
 * Distributed under the MIT license.
 * Copyright (c) 2022 Elton Shih
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "audio_i2s.h"
// #include "wav.h"

#define TRANSFER_RUNS 2000

#define NUM_CHANNELS 2
#define BPS 24
#define SAMPLE_RATE 44100
#define RECORD_DURATION 10 // 5 seconds

#define MAX_18BIT 131071  // Maximum value for signed 18-bit integer


void bin(uint8_t n) {
    uint8_t i;
    // for (i = 1 << 7; i > 0; i = i >> 1)
    //     (n & i) ? printf("1") : printf("0");
    for (i = 0; i < 8; i++) // LSB first
        (n & (1 << i)) ? printf("1") : printf("0");
}

// Function to write 18-bit number into 24-bit audio sample
void write_18bit_sample(FILE *file, uint32_t sample_18bit) {
    uint8_t buffer[4];
    
    sample_18bit = sample_18bit << 6; // now in 24 bit format
    sample_18bit = sample_18bit << 8; // now in 32 bit format

    // sample_18bit ^= 0X80000000;
    sample_18bit -= 0xF8000000;
    // Write 18-bit sample packed into 24 bits
    buffer[0] = (sample_18bit >> 0) & 0xFF;   // Least significant byte (LSB)
    buffer[1] = (sample_18bit >> 8) & 0xFF;   // Middle byte
    buffer[2] = (sample_18bit >> 16) & 0xFF;   // Middle byte
    buffer[3] = (sample_18bit >> 24) & 0xFF;  // Most significant byte

    // Write the 3-byte (24-bit) sample to the file
    fwrite(buffer, sizeof(buffer), 1, file);
}

void parsemem(void* virtual_address, int word_count, FILE *file) {
    uint32_t *p = (uint32_t *)virtual_address;
    // char *b = (char*)virtual_address;
    int offset;

    // uint32_t sample_count = 0;
    uint32_t sample_value = 0;
    uint32_t sample_value_prev = 0;

    for (offset = 0; offset < word_count; offset++) {
        sample_value = p[offset];
        // sample_count = p[offset] >> 18;

        for (int i = 0; i < 4; i++) {
            //bin(b[offset*4+i]);
            //printf(" ");
        }

        //printf(" -> [%d]: %02x (%dp)\n", sample_count, sample_value, sample_value*100/((1<<18)-1));

        // Write the 18-bit sample as 24-bit audio
        if ((offset % 2) == 1) {
            write_18bit_sample(file, sample_value + sample_value_prev); // maybe change later
        }

        // write_18bit_sample(file, sample_value);
        sample_value_prev = sample_value;
    }
}

// Function to write WAV file header
void write_wav_header(FILE *file, int num_samples) {
    int subchunk2_size = num_samples * 4; // 3 bytes per 24-bit sample
    int chunk_size = 36 + subchunk2_size;

    // RIFF header
    fwrite("RIFF", 1, 4, file);
    fwrite(&chunk_size, 4, 1, file);
    fwrite("WAVE", 1, 4, file);

    // fmt subchunk
    fwrite("fmt ", 1, 4, file);
    int subchunk1_size = 16;       // PCM format
    short audio_format = 1;        // PCM
    short num_channels = 1;        // Mono
    int sample_rate = SAMPLE_RATE;
    int byte_rate = SAMPLE_RATE * 4; // 3 bytes per sample for 24-bit PCM
    short block_align = 4;         // 3 bytes per sample
    short bits_per_sample = 32;    // 24 bits per sample

    fwrite(&subchunk1_size, 4, 1, file);
    fwrite(&audio_format, 2, 1, file);
    fwrite(&num_channels, 2, 1, file);
    fwrite(&sample_rate, 4, 1, file);
    fwrite(&byte_rate, 4, 1, file);
    fwrite(&block_align, 2, 1, file);
    fwrite(&bits_per_sample, 2, 1, file);

    // data subchunk
    fwrite("data", 1, 4, file);
    fwrite(&subchunk2_size, 4, 1, file);
}

void write_file(int run, int board) {
    printf("Entered main\n");

    uint32_t frames[TRANSFER_RUNS][TRANSFER_LEN] = {0};

    audio_i2s_t my_config;
    if (audio_i2s_init(&my_config) < 0) {
        printf("Error initializing audio_i2s\n");
        return;
    }

    printf("mmapped address: %p\n", my_config.v_baseaddr);
    printf("Before writing to CR: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_CR));
    audio_i2s_set_reg(&my_config, AUDIO_I2S_CR, 0x1);
    printf("After writing to CR: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_CR));
    printf("SR: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_SR));
    printf("Key: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_KEY));
    printf("Before writing to gain: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_GAIN));
    audio_i2s_set_reg(&my_config, AUDIO_I2S_GAIN, 0x1);
    printf("After writing to gain: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_GAIN));
    
    char filename[50];   // Buffer to hold the filename

    // Construct the filename with the integer
    sprintf(filename, "output%d_%d.wav", run, board);

    // open file
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }
        
    // Calculate number of samples
    uint32_t num_samples = TRANSFER_RUNS * TRANSFER_LEN;      // For simplicity, writing 1 sample

    // Write WAV header
    write_wav_header(file, num_samples);

    printf("Initialized audio_i2s\n");
    printf("Starting audio_i2s_recv\n");

    for (int i = 0; i < TRANSFER_RUNS; i++) {
        int32_t *samples = audio_i2s_recv(&my_config);
        memcpy(frames[i], samples, TRANSFER_LEN*sizeof(uint32_t));
    }

    for (int i = 0; i < TRANSFER_RUNS; i++) {
        //printf("Frame %d:\n", i);
        parsemem(frames[i], TRANSFER_LEN, file);
        //printf("==============================\n");
    }
    
    audio_i2s_release(&my_config);

    fclose(file);
    
    printf("WAV file written successfully!\n");
}

#define PORT 5000         // Port to listen on
#define BUFFER_SIZE 1024  // Buffer size for incoming data
#define START_SIGNAL "START" // The expected start signal
//#define SPECIFIC_IP "192.168.0.10" // Replace with the exact IP you want to allow


int main() {
    int board;
    int server_fd, client_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_addr_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];

    const char *env_var_name = "BOARD"; // The name of the environment variable
    char *env_var_value = getenv(env_var_name); // Get the environment variable as a string

    const char *master_ip_name = "MASTER_IP"; // The name of the environment variable
    char *master_ip_value = getenv(master_ip_name); // Get the environment variable as a string

    if (env_var_value == NULL) {
        printf("Environment variable %s is not set.\n", env_var_name);
        return 1;
    }

    // Convert the string to an integer
    board = atoi(env_var_value); // Use atoi if you are sure the value is a valid integer
    printf("The value of %s as an integer is: %d\n", env_var_name, board);

    // Create the socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(master_ip_value); // Bind only to SPECIFIC_IP
    server_address.sin_port = htons(PORT);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Listening for a connection from IP %s on port %d...\n", master_ip_value, PORT);

    // Accept a single connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_addr_len)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Check if the client IP matches the specific IP
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    if (strcmp(client_ip, master_ip_value) != 0) {
        printf("Connection from unauthorized IP %s. Closing connection.\n", client_ip);
        close(client_fd);
    } else {
        printf("Connected to %s.\n", client_ip);

        // Infinite loop to keep receiving data from this specific client
        while (1) {
            // Clear the buffer before each read
            memset(buffer, 0, BUFFER_SIZE);

            // Read data from the client
            int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1); // Leave space for null terminator
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0'; // Null-terminate the received data
                printf("Received data: %s\n", buffer);
                int number = atoi(buffer);

                // Check if the received data is the "START" signal
                if (number > 0) {
                    printf("START signal received. Proceeding with the next steps...\n");
                    // Add any additional actions you want to perform after receiving "START"
                    write_file(number, board);
                } else {
                    printf("Unexpected data received.\n");
                }
            } else if (bytes_read == 0) {
                // Connection was closed by the client
                printf("Client disconnected. Waiting for reconnection...\n");
                close(client_fd);
                break;
            } else {
                perror("Failed to read data");
                break;
            }
        }
    }

    // Close the server socket
    close(server_fd);
    return 0;
}
