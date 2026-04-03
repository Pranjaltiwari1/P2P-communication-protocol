#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <bitset>
#include <cstdint>

const int SAMPLE_RATE = 44100;
const int BAUD_RATE = 100;
const double FREQ_SPACE = 1200.0; // Binary '0'
const double FREQ_MARK = 2200.0;  // Binary '1'
const double PI = 3.14159265358979323846;

using namespace std;

// --- 1. The Goertzel Algorithm ---
double goertzelMagnitude(const vector<int16_t>& samples, int start_idx, int num_samples,
                         double target_freq) {

    int k = (int)(0.5 + ((num_samples * target_freq) / SAMPLE_RATE));
    double omega = (2.0 * PI * k) / num_samples;
    double sine = sin(omega);
    double cosine = cos(omega);
    double coeff = 2.0 * cosine;

    double q0 = 0, q1 = 0, q2 = 0;

    for (int i = 0; i < num_samples; i++) {
        // Normalize sample from 16-bit integer to -1.0 to 1.0 range
        double sample = samples[start_idx + i] / 32768.0;
        q0 = coeff * q1 - q2 + sample;
        q2 = q1;
        q1 = q0;
    }

    // return magnitude squared
    return (q1 * q1) + (q2 * q2) - (q1 * q2 * coeff);
}

// --- 2. Read WAV File (Basic implementation) ---
vector<int16_t> readWavFile(const string& filename) {
    ifstream file(filename, ios::binary);
    vector<int16_t> audio_data;

    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename
             << ". Did you run the transmitter first?" << endl;
        return audio_data;
    }

    // Skip the 44-byte WAV header to get straight to the raw audio data
    file.seekg(44, ios::beg);

    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(int16_t))) {
        audio_data.push_back(sample);
    }

    file.close();
    return audio_data;
}

// --- 3. Decryption Layer (Placeholder) ---
string decryptMessage(const string& encrypted, char key) {
    string decrypted = "";
    for (char c : encrypted) {
        decrypted += c ^ key; // Reversing the XOR
    }
    return decrypted;
}

// --- Main Pipeline ---
int main() {

    cout << "1. Loading audio file..." << endl;
    vector<int16_t> audioBuffer = readWavFile("secret_transmission.wav");
    if (audioBuffer.empty()) return 1;

    cout << "2. Demodulating audio into bits using Goertzel..." << endl;
    int samples_per_bit = SAMPLE_RATE / BAUD_RATE;
    vector<int> received_bits;

    // Jump through the audio file chunk by chunk (one bit at a time)
    for (size_t i = 0; i + samples_per_bit <= audioBuffer.size(); i += samples_per_bit) {
        double mag_space = goertzelMagnitude(audioBuffer, i, samples_per_bit, FREQ_SPACE);
        double mag_mark  = goertzelMagnitude(audioBuffer, i, samples_per_bit, FREQ_MARK);

        // Whichever frequency is louder is our bit
        if (mag_mark > mag_space) {
            received_bits.push_back(1);
        } else {
            received_bits.push_back(0);
        }
    }

    cout << "3. Decoding bits into bytes..." << endl;
    string extracted_ciphertext = "";
    bool found_start = false;

    // Look for our Start Byte (0xAA or 10101010)
    for (size_t i = 0; i <= received_bits.size() - 8; i++) {
        if (!found_start) {
            if (received_bits[i] == 1 && received_bits[i + 1] == 0 &&
                received_bits[i + 2] == 1 && received_bits[i + 3] == 0 &&
                received_bits[i + 4] == 1 && received_bits[i + 5] == 0 &&
                received_bits[i + 6] == 1 && received_bits[i + 7] == 0) {
                found_start = true;
                i += 7; // Skip the start byte
            }
        } else {
            // Reconstruct the 8-bit character
            char c = 0;
            for (int b = 0; b < 8; b++) {
                c |= (received_bits[i + b] << (7 - b));
            }
            extracted_ciphertext += c;
            i += 7; // Skip to the next byte
        }
    }

    if (!found_start) {
        cout << "FAILED: Could not find the start byte (10101010) in the audio stream." << endl;
        return 1;
    }

    cout << "4. Decrypting message..." << endl;
    char dummyKey = 'K';
    string plaintext = decryptMessage(extracted_ciphertext, dummyKey);

    cout << "\n----------------------" << endl;
    cout << "DECRYPTED MESSAGE: " << plaintext << endl;
    cout << "----------------------" << endl;

    return 0;
}