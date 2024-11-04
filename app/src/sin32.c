#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 44100  // Sample rate in Hz
#define DURATION 5         // Duration in seconds
#define AMPLITUDE 2147483647  // Maximum amplitude for 32-bit signed PCM

// Function to write WAV file header for 32-bit PCM
void write_wav_header(FILE *file, int num_samples) {
    int subchunk2_size = num_samples * 4;  // 4 bytes per 32-bit sample
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
    int byte_rate = SAMPLE_RATE * 4;  // 4 bytes per sample for 32-bit PCM
    short block_align = 4;         // 4 bytes per sample * 1 channel
    short bits_per_sample = 32;    // 32 bits per sample

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

// Function to generate and write 32-bit sine wave samples to the WAV file
void write_sine_wave(FILE *file, double frequency) {
    int num_samples = SAMPLE_RATE * DURATION;

    for (int i = 0; i < num_samples; i++) {
        // Generate the sine wave sample
        double sample_value = AMPLITUDE * sin(2 * M_PI * frequency * i / SAMPLE_RATE);
        int32_t sample = (int32_t)sample_value;  // Convert to 32-bit signed integer

        // Write the sample to the file
        fwrite(&sample, sizeof(int32_t), 1, file);
    }
}

int main() {
    // Open the file for writing
    FILE *file = fopen("sine_wave_32bit.wav", "wb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // Calculate the number of samples
    int num_samples = SAMPLE_RATE * DURATION;

    // Write the WAV header
    write_wav_header(file, num_samples);

    // Write the sine wave data (e.g., 440 Hz for A4 note)
    write_sine_wave(file, 440.0);

    // Close the file
    fclose(file);
    printf("32-bit sine wave written to 'sine_wave_32bit.wav'\n");

    return 0;
}
