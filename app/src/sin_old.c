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
#include <string.h>
#include <stdint.h>

// #include "wav.h"

#define TRANSFER_LEN 256

#define TRANSFER_RUNS 2000

#define NUM_CHANNELS 2
#define BPS 24
#define SAMPLE_RATE 44100
#define RECORD_DURATION 10

void bin(uint8_t n) {
    uint8_t i;
    // for (i = 1 << 7; i > 0; i = i >> 1)
    //     (n & i) ? printf("1") : printf("0");
    for (i = 0; i < 8; i++) // LSB first
        (n & (1 << i)) ? printf("1") : printf("0");
}

// Function to write 18-bit number into 24-bit audio sample
void write_18bit_sample(FILE *file, int32_t sample_18bit) {
    uint8_t buffer[3];
    
    // Write 18-bit sample packed into 24 bits
    buffer[0] = (sample_18bit >> 17) & 0xFF;  // Most significant byte
    buffer[1] = (sample_18bit >> 9) & 0xFF;   // Middle byte
    buffer[2] = (sample_18bit >> 1) & 0xFF;   // Least significant byte (LSB)

    // Write the 3-byte (24-bit) sample to the file
    fwrite(buffer, sizeof(buffer), 1, file);
}

void parsemem(void* virtual_address, int word_count, FILE *file) {
    uint32_t *p = (uint32_t *)virtual_address;
    char *b = (char*)virtual_address;
    int offset;

    uint32_t sample_count = 0;
    uint32_t sample_value = 0;
    uint32_t sample_value_prev = 0;

    for (offset = 0; offset < word_count; offset++) {
        sample_value = p[offset] & ((1<<18)-1);
        sample_count = p[offset] >> 18;

        for (int i = 0; i < 4; i++) {
            //bin(b[offset*4+i]);
            //printf(" ");
        }

        //printf(" -> [%d]: %02x (%dp)\n", sample_count, sample_value, sample_value*100/((1<<18)-1));

        // Write the 18-bit sample as 24-bit audio
        if ((offset % 2) == 1) {
            write_18bit_sample(file, sample_value + sample_value_prev); // if odd write
        }
        // write_18bit_sample(file, sample_value);
        sample_value_prev = sample_value;
    }

}

#pragma pack(push, 1)
typedef struct {
    // RIFF Header
    char riff[4];            // 'RIFF'
    uint32_t chunk_size;      // File size - 8 bytes
    char wave[4];            // 'WAVE'

    // Format Sub-chunk
    char fmt[4];              // 'fmt '
    uint32_t subchunk1_size;  // Size of format subchunk (16 for PCM)
    uint16_t audio_format;    // Audio format (1 for PCM)
    uint16_t num_channels;    // Number of channels
    uint32_t sample_rate;     // Sample rate (e.g., 44100 Hz)
    uint32_t byte_rate;       // Byte rate = SampleRate * NumChannels * BitsPerSample / 8
    uint16_t block_align;     // Block align = NumChannels * BitsPerSample / 8
    uint16_t bits_per_sample; // Bits per sample (24 for this case)

    // Data Sub-chunk
    char data[4];             // 'data'
    uint32_t subchunk2_size;  // Size of the data (NumSamples * NumChannels * BitsPerSample / 8)
} WAVHeader;
#pragma pack(pop)

// Function to write the WAV file header
void write_wav_header(FILE *file, uint32_t sample_rate, uint32_t num_samples) {
    WAVHeader header;
    
    // RIFF Chunk
    header.riff[0] = 'R';
    header.riff[1] = 'I';
    header.riff[2] = 'F';
    header.riff[3] = 'F';
    header.chunk_size = 36 + num_samples * 3;  // 36 + data chunk size (3 bytes per sample in 24-bit)
    header.wave[0] = 'W';
    header.wave[1] = 'A';
    header.wave[2] = 'V';
    header.wave[3] = 'E';
    
    // Format Subchunk
    header.fmt[0] = 'f';
    header.fmt[1] = 'm';
    header.fmt[2] = 't';
    header.fmt[3] = ' ';
    header.subchunk1_size = 16;  // PCM size
    header.audio_format = 1;     // PCM
    header.num_channels = 1;     // Mono
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * 3;  // SampleRate * NumChannels * BitsPerSample / 8
    header.block_align = 3;      // NumChannels * BitsPerSample / 8
    header.bits_per_sample = 24; // 24 bits per sample
    
    // Data Subchunk
    header.data[0] = 'd';
    header.data[1] = 'a';
    header.data[2] = 't';
    header.data[3] = 'a';
    header.subchunk2_size = num_samples * 3;  // NumSamples * NumChannels * BitsPerSample / 8
    
    // Write header to file
    fwrite(&header, sizeof(WAVHeader), 1, file);
}

int main() {
    printf("Entered main\n");
    
    // open file
    FILE *file = fopen("output.wav", "wb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }
    
    uint32_t sample_rate = 44100;  // Sample rate (e.g., 44100 Hz)
    uint32_t num_samples = TRANSFER_RUNS * TRANSFER_LEN;      // For simplicity, writing 1 sample
    
    // Write WAV header
    write_wav_header(file, sample_rate, num_samples);
    
    // Example 18-bit number (you can modify this as needed)
    int32_t data_18bit;  // A 18-bit number (262143 in decimal)

    for (int i = 0; i < num_samples; ++i) {
        
    }

    printf("Initialized audio_i2s\n");
    printf("Starting audio_i2s_recv\n");

    fclose(file);
    
    printf("WAV file written successfully!\n");
    return 0;
}

#include <stdio.h>
#include <math.h>

#define PI 3.14159265
#define MAX_18BIT 131071  // Maximum value for signed 18-bit integer
#define MIN_18BIT -131072 // Minimum value for signed 18-bit integer

// Function to generate the sine wave sample in 18-bit format
int generate_sine_wave_18bit(int i, double amplitude, double frequency, double sample_rate) {
    // Generate the sine wave sample
    double raw_value = amplitude * sin(2 * PI * frequency * i / sample_rate);

    // Scale the amplitude to fit within the 18-bit range
    int scaled_value = (int)(raw_value * MAX_18BIT);

    // Clamp the value to be within the 18-bit range
    if (scaled_value > MAX_18BIT) {
        scaled_value = MAX_18BIT;
    } else if (scaled_value < MIN_18BIT) {
        scaled_value = MIN_18BIT;
    }

    return scaled_value;
}

int main() {
    double amplitude = 1.0;      // Maximum amplitude (use 1.0 to fit in full range)
    double frequency = 440.0;    // Frequency in Hz (e.g., 440 Hz is the A4 note)
    double sample_rate = 44100.0; // Typical audio sample rate in samples per second

    // Generate and print 100 samples of the sine wave in 18-bit format
    for (int i = 0; i < 100; i++) {
        int sample = generate_sine_wave_18bit(i, amplitude, frequency, sample_rate);
        printf("Sample %d: %d\n", i, sample);
    }

    return 0;
}