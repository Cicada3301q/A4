#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

struct WAVHeader {
    char chunkID[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    // char subchunk2ID[4];
    // uint32_t subchunk2Size;
};

void printWAVHeader(const WAVHeader& header) {
    std::cout << "chunkID: " << header.chunkID[0] << header.chunkID[1] << header.chunkID[2] << header.chunkID[3] << std::endl;
    std::cout << "chunkSize: " << header.chunkSize << std::endl;
    std::cout << "format: " << header.format[0] << header.format[1] << header.format[2] << header.format[3] << std::endl;
    std::cout << "subchunk1ID: " << header.subchunk1ID[0] << header.subchunk1ID[1] << header.subchunk1ID[2] << header.subchunk1ID[3] << std::endl;
    std::cout << "subchunk1Size: " << header.subchunk1Size << std::endl;
    std::cout << "audioFormat: " << header.audioFormat << std::endl;
    std::cout << "numChannels: " << header.numChannels << std::endl;
    std::cout << "sampleRate: " << header.sampleRate << std::endl;
    std::cout << "byteRate: " << header.byteRate << std::endl;
    std::cout << "blockAlign: " << header.blockAlign << std::endl;
    std::cout << "bitsPerSample: " << header.bitsPerSample << std::endl;
    // std::cout << "subchunk2ID: " << header.subchunk2ID[0] << header.subchunk2ID[1] << header.subchunk2ID[2] << header.subchunk2ID[3] << std::endl;
    // std::cout << "subchunk2Size: " << header.subchunk2Size << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " inputfile outputfile" << std::endl;
        return 1;
    }

    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];

    std::ifstream inFile(inputFileName, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error opening input file: " << inputFileName << std::endl;
        return 1;
    }

    WAVHeader header;
    inFile.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    printWAVHeader(header); // Print the header content

    // Writing the header to the output file
    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFileName << std::endl;
        inFile.close();
        return 1;
    }
    

    outFile.write(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (header.subchunk1Size != 16) {
    std::cout << "subchunk1 is larger than expected. Skipping extra bytes!" << std::endl;

    // Calculate the number of extra bytes to skip
    int remainder = header.subchunk1Size - 16;

    // Create a buffer to read and write the extra bytes
    std::vector<char> extraData(remainder);

    // Read the extra bytes from input file
    inFile.read(extraData.data(), remainder);

    // Write the extra bytes to output file
    outFile.write(extraData.data(), remainder);
}

    char subchunk2_id[4];
    int subchunk2_size;
    inFile.read(subchunk2_id, sizeof(subchunk2_id));
    inFile.read(reinterpret_cast<char*>(&subchunk2_size), sizeof(subchunk2_size));
    outFile.write(subchunk2_id, sizeof(subchunk2_id));
    outFile.write(reinterpret_cast<char*>(&subchunk2_size), sizeof(subchunk2_size));
    std::cout << "subchunk2_id: ";
    for (int i = 0; i < 4; ++i) {
        std::cout << subchunk2_id[i];
    }
    std::cout << std::endl;

// Printing subchunk2_size
std::cout << "subchunk2_size: " << subchunk2_size << std::endl;
    // Read and write audio data
    std::vector<char> buffer(subchunk2_size);
    inFile.read(buffer.data(), subchunk2_size);
    outFile.write(buffer.data(), subchunk2_size);

    // Close files
    inFile.close();
    outFile.close();

    std::cout << "WAV file read from " << inputFileName << " and written to " << outputFileName << std::endl;

    return 0;
}