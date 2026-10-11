#ifndef CMR2_CATEGORY_BEST_RESULTS_H
#define CMR2_CATEGORY_BEST_RESULTS_H

// Best results begin at +0x84 in each 0x650-byte save profile. The
// frontend merges these entries and RallyData displays their times.
// These are packed integer words, never native addresses.
struct CategoryBestTime {
    unsigned int header; // car, transmission, present bit, random tag
    unsigned int time;
};

struct CategoryBestResult {
    unsigned int header;
    unsigned int result; // mode-specific packed difficulty/position fields
    int extra;
};

struct CategoryPlayerResult {
    CategoryBestResult result;
    int field_0xc;
};

struct CategoryBestResults {
    CategoryPlayerResult playerResults[3];       // 0x000
    CategoryBestResult rallyResults[8][3];      // 0x030 country/difficulty
    CategoryBestTime rallyStageTimes[8][12];    // 0x150 country/stage
    CategoryBestResult arcadeResults[3][3];     // 0x450 group/difficulty
    CategoryBestTime arcadeStageTimes[3][3];    // 0x4bc group/stage
};

unsigned char Frontend_MergeBestDeviceStageOption(int index, CategoryBestResults *pBlock);
unsigned char Frontend_MergeBestDeviceButtonOption(int index, CategoryBestResults *pBlock);
unsigned char Frontend_MergeBestPlayerStageOption(int index, CategoryBestResults *pBlock);
unsigned char Frontend_MergeBestSecondaryPlayerOption(int index, CategoryBestResults *pBlock);

#endif
