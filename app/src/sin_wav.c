#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265
#define MAX_18BIT 131071  // Maximum value for signed 18-bit integer
#define SAMPLE_RATE 44100 // Sampling rate in Hz
#define DURATION 5        // Duration in seconds

// Function to write a 24-bit integer to a file (little endian)
void write_24bit(FILE *file, int value) {
    fputc(value & 0xFF, file);
    fputc((value >> 8) & 0xFF, file);
    fputc((value >> 16) & 0xFF, file);

    printf("0x ");
    printf("%02X ", (value >> 16) & 0xFF);
    printf("%02X ", (value >> 8) & 0xFF);
    printf("%02X ", value & 0xFF);
    printf("\n");
}

// Function to generate the sine wave and write to file
void write_sine_wave(FILE *file, double amplitude, double frequency) {
    int num_samples = SAMPLE_RATE * DURATION;

    for (int i = 0; i < num_samples; i++) {
        // Generate sine wave sample
        double raw_value = amplitude * sin(2 * PI * frequency * i / SAMPLE_RATE);
        
        // Scale to 18-bit range and mask to fit within 24-bit container
        int sample = (int)(raw_value * MAX_18BIT);  // Mask lower 6 bits

        // Write 24-bit PCM data to file (for a mono channel)
        write_24bit(file, sample);
    }
}

// Function to write WAV file header
void write_wav_header(FILE *file, int num_samples) {
    int subchunk2_size = num_samples * 3; // 3 bytes per 24-bit sample
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
    int byte_rate = SAMPLE_RATE * 3; // 3 bytes per sample for 24-bit PCM
    short block_align = 3;         // 3 bytes per sample
    short bits_per_sample = 24;    // 24 bits per sample

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

int main() {
    // Open file for binary writing
    FILE *file = fopen("sin.wav", "wb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // Calculate number of samples
    int num_samples = SAMPLE_RATE * DURATION;

    // Write WAV header
    write_wav_header(file, num_samples);

    // Write sine wave data to file
    double amplitude = 1.0;    // Maximum amplitude
    double frequency = 440.0;  // Frequency of sine wave in Hz (e.g., A4 note)
    write_sine_wave(file, amplitude, frequency);

    // Close the file
    fclose(file);
    printf("18-bit PCM sine wave written to 'sine_wave_18bit.wav' (stored as 24-bit PCM)\n");

    return 0;
}
