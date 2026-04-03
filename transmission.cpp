#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <bitset>
#include <cstdint>

using namespace std;

// --- Configuration Constants ---
const int SAMPLE_RATE = 44100;      // Standard CD-quality audio
const int BAUD_RATE = 100;          // Bits per second
const double FREQ_SPACE = 1200.0;   // Frequency for binary '0'
const double FREQ_MARK = 2200.0;    // Frequency for binary '1'
const double AMPLITUDE = 0.8;       // Volume (0.0 to 1.0)
const double PI = 3.14159265358979323846;


// --- 1. Cryptography Layer (Placeholder) ---
string encryptMessage(const string& message, char key) {
    string encrypted = "";
    for (char c : message) {
        encrypted += c ^ key;   // Simple XOR encryption (demo only)
    }
    return encrypted;
}


// --- 2. String to Binary Converter ---
vector<int> stringToBits(const string& text) {
    vector<int> bits;

    // Start byte (10101010)
    uint8_t start_byte = 0xAA;
    for (int i = 7; i >= 0; --i)
        bits.push_back((start_byte >> i) & 1);

    for (char c : text) {
        bitset<8> b(c);
        for (int i = 7; i >= 0; --i) {
            bits.push_back(b[i]);
        }
    }
    return bits;
}


// --- 3. AFSK Modulator ---
vector<int16_t> modulateBitsToAudio(const vector<int>& bits) {
    vector<int16_t> audio_samples;
    double phase = 0.0;
    int samples_per_bit = SAMPLE_RATE / BAUD_RATE;

    for (int bit : bits) {

        double frequency = (bit == 1) ? FREQ_MARK : FREQ_SPACE;
        double phase_increment = 2.0 * PI * frequency / SAMPLE_RATE;

        for (int i = 0; i < samples_per_bit; ++i) {

            double sample = AMPLITUDE * sin(phase);
            audio_samples.push_back(static_cast<int16_t>(sample * 32767));

            phase += phase_increment;
            if (phase >= 2.0 * PI)
                phase -= 2.0 * PI;
        }
    }

    return audio_samples;
}


// --- 4. WAV File Exporter ---
void writeWavFile(const string& filename, const vector<int16_t>& data) {
    ofstream file(filename, ios::binary);

    int32_t dataSize = data.size() * sizeof(int16_t);
    int32_t fileSize = 36 + dataSize;

    file << "RIFF";
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file << "WAVEfmt ";

    int32_t fmtSize = 16;
    int16_t audioFormat = 1;
    int16_t numChannels = 1;
    int32_t sampleRate = SAMPLE_RATE;
    int32_t byteRate = SAMPLE_RATE * sizeof(int16_t);
    int16_t blockAlign = sizeof(int16_t);
    int16_t bitsPerSample = 16;

    file.write(reinterpret_cast<const char*>(&fmtSize), 4);
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    file << "data";
    file.write(reinterpret_cast<const char*>(&dataSize), 4);
    file.write(reinterpret_cast<const char*>(data.data()), dataSize);

    file.close();

    cout << "Audio saved to " << filename << endl;
}


// --- Main Pipeline ---
int main() {

    string secretMessage = "HELLO FROM GEMINI";
    char dummyKey = 'K';

    cout << "1. Encrypting message..." << endl;
    string encryptedText = encryptMessage(secretMessage, dummyKey);

    cout << "2. Converting to binary stream..." << endl;
    vector<int> bitStream = stringToBits(encryptedText);

    cout << "3. Modulating binary into audio frequencies..." << endl;
    vector<int16_t> audioBuffer = modulateBitsToAudio(bitStream);

    cout << "4. Exporting to WAV..." << endl;
    writeWavFile("secret_transmission.wav", audioBuffer);

    return 0;
}