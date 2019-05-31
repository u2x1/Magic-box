#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include <bitset>

using namespace std;

void encode(string &contain, string &secret);
void decode(string &contain);

string readFile(char path[]);
void writeFile(string& content, string file_name);

size_t findNextFrame(string &content, size_t offset);
size_t getFrameLen(const string &content, size_t headerPos);

string cp_cr8_header(string &content, int oriBitrate, int oriSamprate);

unsigned int getSample(const string &content, size_t pos);
unsigned int getBitrate(const string &content, size_t pos);
void writeSample(unsigned int bitrate, string &content, size_t pos);

int main(int argc, char* argv[]){
    if(argc == 3){
        string contain = readFile(argv[1]);
        string secret = readFile(argv[2]);
        string scr_ext = string(argv[2]).substr(string(argv[2]).find_last_of('.') + 1);
        secret.insert(0, to_string(scr_ext.size()) + scr_ext);
        encode(contain, secret);
        return 0;
    }
    else if(argc == 2){
        string contain = readFile(argv[1]);
        decode(contain);
        return 0;
    }
    cout << "error: Invalid argument." << endl;
    cout << "[Usage:\thider carrier_file secret_file\n\thider carrier_file]" << endl;
	return 1;
}

void encode(string &contain, string &secret){
    secret.insert(0, "^:_;");
    secret += "!@*%";

	// Skiping the id3v2 tag
	size_t pos = ((contain[6] & 0x7F) << 21) + ((contain[7] & 0x7F) << 14) + ((contain[8] & 0x7F) << 7) + (contain[9] & 0x7F) + 10;
	// Skip info tag.
    for(int i = 0; i < 3; ++i)
        pos = findNextFrame(contain, pos + 1);
    // for(int i = 0; i < 100; ++i){
    string header = cp_cr8_header(contain, getBitrate(contain, pos), getSample(contain, pos));

    size_t len = getFrameLen(header, 0) - 4;
    size_t process = 0;
	while(process <= secret.size()){
        string frame = header;
        if((secret.size() - process) / len){
            frame += secret.substr(process, len);
        }
        else{
            frame += secret.substr(process, secret.size() - process);
            frame += string(len - secret.size() + process, '\0');
        }
        contain.insert(pos, frame);
        pos += len + 4;
        process += len;
	}
	writeFile(contain, "output.mp3");
}

void decode(string &contain){
    // Skiping the id3v2 tag
	size_t pos = ((contain[6] & 0x7F) << 21) + ((contain[7] & 0x7F) << 14) + ((contain[8] & 0x7F) << 7) + (contain[9] & 0x7F) + 10;
	// Skip info tag.
    for(int i = 0; i < 3; ++i)
        pos = findNextFrame(contain, pos + 1);
    string secret;
    size_t len = getFrameLen(contain, pos) - 4;
    while(pos != string::npos){
        string frame = contain.substr(pos + 4, len);
        if(frame.find("^:_;") != string::npos)
            break;
        pos = findNextFrame(contain, pos + len + 4);
    }

    while(pos != string::npos){
        string frame = contain.substr(pos + 4, len);
        secret += frame;
        if(frame.find("!@*%") != string::npos)
            break;
        pos = findNextFrame(contain, pos + len + 4);
    }
    secret.erase(0, secret.find("^:_;") + 4);
    secret.erase(secret.find("!@*%"), secret.find("!@*%") + 4 - secret.size());
    string ext = secret.substr(1, secret[0] - '0');
    secret.erase(0, secret[0] - '0' + 1);
    writeFile(secret, "secret." + ext);
}

string readFile(char path[]) {
    FILE* pFile = fopen(path, "rb");
	string content;
    if (pFile) {
        size_t fsize;
        fseek(pFile, 0, SEEK_END);
        fsize = static_cast<size_t>(ftell(pFile));
        rewind(pFile);
        content = string(fsize, '\0');
        fread(&content[0], 1, fsize, pFile);
        fclose(pFile);
    }
    else
        cout << "error: Open file failed." << endl;
	return content;
}

void writeFile(string& content, string file_name) {
    FILE* output = fopen(file_name.c_str(), "wb");
    if(output) {
        fwrite(static_cast<char*>(&content[0]), 1, content.size(), output);
        fclose(output);
        cout << "File has been saved as: " << file_name << endl;
    }
    else
        cout << "error: Write file failed." << endl;
}

size_t findNextFrame(string &carry, size_t offset = 0) {
    size_t pos = carry.find(char(0xff), offset);
    while (pos != string::npos) {
        if (carry[pos + 1] == char(0xfa) || carry[pos + 1] == char(0xfb))
            break;
        pos = carry.find(char(0xff), pos + 1);
    }
    return pos;
}
size_t getFrameLen(const string &content, size_t headerPos = 0){
    return getBitrate(content, headerPos) * 144 / getSample(content, headerPos);
}
unsigned int getSample(const string &content, size_t pos = 0) {
    pos += 2;
    unsigned long long i = static_cast<unsigned long long>(content[pos]);
    bitset<2> bs{ (i >> 6) };
    unsigned int bitrate = bs.to_ulong();
    return bitrate;
}

unsigned int getBitrate(const string &content, size_t pos = 0) {
    pos += 2;
    unsigned long long i = static_cast<unsigned long long>(content[pos]);
    bitset<4> bs{ (i >> 4) };
    unsigned int bitrate = bs.to_ulong();
    return bitrate;
}

// Create a frame that is unavailable to play: with bad bit rate and reserved sample rate
string cp_cr8_header(string &content, int oriBitrate, int oriSamprate){
	size_t pos = ((content[6] & 0x7F) << 21) + ((content[7] & 0x7F) << 14) + ((content[8] & 0x7F) << 7) + (content[9] & 0x7F) + 10;
    // Skip info tag
    for(int i = 0; i < 2; ++i)
        pos = findNextFrame(content, pos + 1);
    string header = content.substr(pos, 4);
    header[2] |= 0xfc;
    return header;
}

void writeSample(unsigned int sample, string &content, size_t pos) {
    pos += 2;
    unsigned long long i = static_cast<unsigned long long>(content[pos]);
    bitset<8> bs{ i };
    bitset<4> br{ sample };
    for (size_t i = 7; i != 3; --i)
        bs[i] = br[i - 4];
    char br_ch = static_cast<char>(bs.to_ulong());
    content[pos] = br_ch;
}
