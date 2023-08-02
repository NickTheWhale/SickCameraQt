#include <random>
#include <iostream>
#include <algorithm>
#include <vector>

int main() {
    const int FRAME_SIZE = 20;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0, std::numeric_limits<uint16_t>::max());

    // fill FrameType with data that may contain 0
    std::vector<uint16_t> FrameType;
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        auto rand = static_cast<uint16_t>(dist(mt));
        if (rand < std::numeric_limits<uint16_t>::max() / 5)
            rand = 0;
        FrameType.push_back(rand);
    }

    // fill mask 
    std::vector<bool> mask;
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        auto bit = true;
        auto rand = static_cast<uint16_t>(dist(mt));
        if (rand < std::numeric_limits<uint16_t>::max() / 2)
            bit = false;
        mask.push_back(bit);
    }

    // apply mask
    std::vector<uint16_t> maskedFrame;
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        uint16_t val;
        if (mask[i])
            val = FrameType[i];
        else
            val = 0;
        maskedFrame.push_back(val);
    }

    std::cout << "FrameType\t\|\tmask\t|\tmaskedFrame\n";
    for (int i = 0; i < FRAME_SIZE; ++i)
    {
        std::cout << FrameType[i] << "\t|\t" << mask[i] << "\t|\t" << maskedFrame[i] << "\n";
    }
}