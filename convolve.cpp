#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

struct WavHeader {
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
};

void printWAVHeader(const WavHeader& header) {
    std::cout << "chunkID: " << header.chunk_id[0] << header.chunk_id[1] << header.chunk_id[2] << header.chunk_id[3] << std::endl;
    std::cout << "chunkSize: " << header.chunk_size << std::endl;
    std::cout << "format: " << header.format[0] << header.format[1] << header.format[2] << header.format[3] << std::endl;
    std::cout << "subchunk1ID: " << header.subchunk1_id[0] << header.subchunk1_id[1] << header.subchunk1_id[2] << header.subchunk1_id[3] << std::endl;
    std::cout << "subchunk1Size: " << header.subchunk1_size << std::endl;
    std::cout << "audioFormat: " << header.audio_format << std::endl;
    std::cout << "numChannels: " << header.num_channels << std::endl;
    std::cout << "sampleRate: " << header.sample_rate << std::endl;
    std::cout << "byteRate: " << header.byte_rate << std::endl;
    std::cout << "blockAlign: " << header.block_align << std::endl;
    std::cout << "bitsPerSample: " << header.bits_per_sample << std::endl;
    // std::cout << "subchunk2ID: " << header.subchunk2_id[0] << header.subchunk2_id[1] << header.subchunk2_id[2] << header.subchunk2_id[3] << std::endl;
    // std::cout << "subchunk2Size: " << header.subchunk2_size << std::endl;
}

/*
    The function convolve takes six arguments: 
        Two input arrays x[] and h[], their respective sizes N and M, and an output array y[] with size P.

    The first loop initializes the output array y[] to zero. 
        This is necessary because the convolution operation involves accumulating values in y[].

    The second loop (outer loop) iterates over each element of the input array x[].

    The third loop (inner loop) iterates over each element of the array h[]. 
        For each pair of elements x[n] and h[m], it adds their sum to the corresponding element in y[].
*/
void convolve(float x[], int N, float h[], int M, float y[], int P)
{
    int n,m;

    /* Clear Output Buffer y[] */
    for (n=0; n < P; n++)
    {
        y[n] = 0.0;
    }

    /* Outer Loop: process each input value x[n] in turn */
    for (n=0; n<N; n++){
        /* Inner loop: process x[n] with each sample of h[n] */
        for (m=0; m<M; m++){
            y[n+m] += x[n] * h[m];
        }
    }
}

float bytesToFloat(short s) {
    return static_cast<float>(s) / 32768.0f;
}


void readTone(const char *sampleTone, const char *impulseTone, const char *outputFile) {
    std::cout << "in read" << std::endl;
    std::ifstream sampleFileStream(sampleTone, std::ios::binary);
    std::ifstream impulseFileStream(impulseTone, std::ios::binary);
    std::ofstream outputFileStream(outputFile, std::ios::binary);
    std::cout << "past if" << std::endl;
    WavHeader header_sample;
    WavHeader header_impulse;


    // Read the header subchunk 1 and write the header into a new file
    sampleFileStream.read(reinterpret_cast<char*>(&header_sample), sizeof(header_sample));
    impulseFileStream.read(reinterpret_cast<char*>(&header_impulse), sizeof(header_impulse));


    outputFileStream.write(reinterpret_cast<char*>(&header_sample), sizeof(WavHeader));
    std::cout << "Sample Tone Header:" << std::endl;
    printWAVHeader(header_sample);

    std::cout << "\nImpulse Tone Header:" << std::endl;
    printWAVHeader(header_impulse);
    if (header_sample.subchunk1_size != 16) {
          std::cout << "subchunk1 is larger than expected. Skipping extra bytes!" << std::endl;
        
            // Calculate the number of extra bytes to skip
            int remainder = header_sample.subchunk1_size - 16;
            
            // Create a buffer to read and write the extra bytes
            std::vector<char> extraData(remainder);
            
            // Read the extra bytes from input file
            sampleFileStream.read(extraData.data(), remainder);
            
            // Write the extra bytes to output file
            outputFileStream.write(extraData.data(), remainder);
        }

    if (header_impulse.subchunk1_size != 16) {
        std::cout << "subchunk1 is larger than expected. Skipping extra bytes!" << std::endl;
        
        // Calculate the number of extra bytes to skip
        int remainder = header_impulse.subchunk1_size - 16;
        
        // Create a buffer to read and write the extra bytes
        std::vector<char> extraData(remainder);
        
        // Read the extra bytes from input file
        impulseFileStream.read(extraData.data(), remainder);
    }

    char subchunk2_id_sample[4];
    char subchunk2_id_impulse[4];
    int subchunk2_size_sample; // an integer is 4 bytes
    int subchunk2_size_impulse; // an integer is 4 bytes

    sampleFileStream.read(subchunk2_id_sample, sizeof(subchunk2_id_sample));
    sampleFileStream.read(reinterpret_cast<char*>(&subchunk2_size_sample), sizeof(subchunk2_size_sample));
    impulseFileStream.read(subchunk2_id_impulse, sizeof(subchunk2_id_impulse));
    impulseFileStream.read(reinterpret_cast<char*>(&subchunk2_size_impulse), sizeof(subchunk2_size_impulse));
    // Writing subchunk2_id and subchunk2_size to outputFileStream
    outputFileStream.write(subchunk2_id_sample, sizeof(subchunk2_id_sample));
    outputFileStream.write(reinterpret_cast<char*>(&subchunk2_size_sample), sizeof(subchunk2_size_sample));

        std::cout << "subchunk2_id: ";
        for (int i = 0; i < 4; ++i) {
            std::cout << subchunk2_id_sample[i];
        }
    std::cout << std::endl;

    // Printing subchunk2_size
    std::cout << "subchunk2_size: " << subchunk2_size_sample << std::endl;

    int num_samples = subchunk2_size_sample / (header_sample.bits_per_sample / 8);
    int num_impulse = subchunk2_size_impulse / (header_impulse.bits_per_sample / 8);


    // Audio vectors for convolution
    const int bytesPerSample = header_sample.bits_per_sample / 8;
    std::vector<short> sampleShortData(num_samples);
    std::vector<float> sampleFloatData(num_samples);
    // Read sample audio data
    sampleShortData.resize(num_samples);
    sampleFloatData.resize(num_samples);
    // Assuming each audio sample is represented by 2 bytes (16-bit)
    sampleFileStream.read(reinterpret_cast<char*>(sampleShortData.data()), num_samples * sizeof(short));
    for (int i = 0; i < num_samples; i++) {
        // std::cout << "Sample Data :" << sampleShortData[i] << std::endl;
        sampleFloatData[i] = sampleShortData[i]/32768.0f;
        // std::cout << "Read tone after:" << sampleFloatData[i] << std::endl;
    }

    // // Read impulse audio data
    std::vector<float> impulseData(num_impulse);
    std::vector<short> impulseShortData(num_impulse);
    impulseData.resize(num_impulse);
    impulseShortData.resize(num_impulse);
    impulseFileStream.read(reinterpret_cast<char*>(impulseShortData.data()), num_impulse * sizeof(short));
    for (int i = 0; i < num_impulse; i++) {
        // std::cout << "impulse data:" << impulseShortData[i] << std::endl;
        impulseData[i] = impulseShortData[i]/32768.0f;
        // std::cout << "impulse data after :" << impulseData[i] << std::endl;
    }
    // Perform convolution
    int convolvedSize = num_samples + num_impulse - 1;
    std::vector<float> convolvedData(convolvedSize);
   convolve(sampleFloatData.data(), num_samples, impulseData.data(), num_impulse, convolvedData.data(), convolvedSize);

    std::vector<short> convolvedShortData(convolvedData.size());

    // Convert float data to shorts
    for (size_t i = 0; i < convolvedData.size(); ++i) {
        float sample = convolvedData[i];
        // std::cout << "Sample :" << convolvedData[i] << std::endl;
        // Assuming sample is in the range [-1.0, 1.0], scale it back to [-32768, 32767]
        short convertedSample = static_cast<short>(sample * 32767.0f);
        // Store the converted sample in the short array
        convolvedShortData[i] = convertedSample;
        // std::cout << "Read tone before :" << convolvedShortData[i] << std::endl;
    }


    // outputFileStream.write(reinterpret_cast<char*>(&convolvedSizeInBytes), sizeof(int));
    std::vector<char> buffer(subchunk2_size_sample);
    // Write convolved audio data
    outputFileStream.write(reinterpret_cast<char*>(convolvedShortData.data()), subchunk2_size_sample);

}


int main(int argc, char *argv[]) {
    const char *sampleTone = nullptr;
    const char *impulseTone = nullptr;
    const char *outputFile = nullptr;

    /* Process the command line arguments */
    if (argc == 4) {
        /* Set pointers to the input filenames */
        sampleTone = argv[1];
        impulseTone = argv[2];
        outputFile = argv[3];

    } else {
        /* The user did not supply the correct number of command-line arguments */
        std::cerr << "Usage: " << argv[0] << " sampleTone impulseTone" << std::endl;
        return -1;
    }
    std::cout << "Read tone:" << std::endl;
    readTone(sampleTone, impulseTone, outputFile);
    return 0;
}