#include "../include/wave.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void write_little_endian_value(uint32_t value, int num_bytes, FILE *wav_file) {
    unsigned char byte;
    while (num_bytes > 0) {
        byte = value & 0xff;                    // 0xff:11111111 get the lowest 8 bits of the value
        fwrite(&byte, 1, 1, wav_file);          // write this byte to the file
        value >>= 8;                            // delete the byte that was just writtrn to the file
        num_bytes--;
    }
}

void initialize_wav_header(FILE *wav_file, uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t num_samples) {
    WAV_Header header;
    memcpy(header.chunk_id, "RIFF", 4);         // first 4 bytes of the WAV file is RIFF
    // calculate the file size: 36(everything but the data) + the size of data(in bytes)
    header.file_size = 36 + num_samples * num_channels * (bits_per_sample / 8);
    memcpy(header.format, "WAVE", 4);           // next 4 bytes is WAVE
    memcpy(header.fmt_chunk_marker, "fmt ", 4); // next 4 bytes is fmt
    
    header.format_length = 16;                  // size of the Pulse Code Modulation data format chunk
    header.format_type = 1;                     // the format type is Pulse Code Modulation
    header.num_channels = num_channels;         // set the number of channels
    header.sample_rate = sample_rate;           // set the number of samples per second
                                                // calculate the number of bytes per second 
    header.byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
                                                // calculate the size of one data block
    header.block_align = num_channels * (bits_per_sample / 8);
    header.bits_per_sample = bits_per_sample;   // set the bit depth of each sample

    memcpy(header.data_chunk_header, "data", 4);// next 4 bytes is data
    // calculate the size of actual data(in byte): total number of sample * number of channels * size of each sample(in byte)
    header.data_size = num_samples * num_channels * (bits_per_sample / 8);
                                                // write WAV_Header structure into the file
    fwrite(&header, sizeof(WAV_Header), 1, wav_file);
}

void save_wav_file(const char *filename, uint32_t num_samples, uint32_t *data, uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample) {
    FILE *wav_file = fopen(filename, "wb");     // open and write the file for binary
    if (!wav_file) {                            // if the file can't open print the error
        fprintf(stderr, "Error: Can't open the file '%s' for binary writing\n", filename);
        return;
    }
    // use the function to write the wav file header to the file
    initialize_wav_header(wav_file, sample_rate, num_channels, bits_per_sample, num_samples);

    for (uint32_t i = 0; i < num_samples; ++i) {
        write_little_endian_value(data[i], bits_per_sample / 8, wav_file);  // use the function write the data (each sample)
    }

    fclose(wav_file);                           // close the file
}