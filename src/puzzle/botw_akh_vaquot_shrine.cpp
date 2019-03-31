#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cassert>

struct Point {
    int x;
    int y;
};

enum class FanDirection {
    north,
    east,
    south,
    west,
};

const char* fanDirectionName(FanDirection fd) {
    switch (fd) {
    case FanDirection::north:
        return "north";
    case FanDirection::east:
        return "east";
    case FanDirection::south:
        return "south";
    case FanDirection::west:
        return "west";
    default:
        abort();
    }
}

// Return the direction a fan is pointing in this problem state
FanDirection getFanDirection(uint64_t fanIndex, uint64_t state) {
    return static_cast<FanDirection>((state >> (fanIndex * 2)) & 3);
}

// Return the number of unique states that exist for a problem with this many fans
uint64_t uniqueStateCount(uint64_t fanCount) {
    return 1ull << (fanCount * 2);
}

int main(int argc, const char** argv) {
    static constexpr int height = 4;
    static constexpr int width = 5;
    static constexpr uint64_t fanCount = 6;

    // An array of fan locations
    Point fans[fanCount] = {
        { 0, 0 },
        { 0, 3 },
        { 2, 2 },
        { 3, 1 },
        { 3, 3 },
        { 4, 0 },
    };

    // A 2-d array of locations that are spinning, either because they are fans, or because
    // they are being blown by a fan
    bool spinning[height][width];

    // Each state represents a particular configuration of fan orientations. Brute force all
    // of the configurations until a solution is found.
    for (uint64_t state = 0; state < uniqueStateCount(fanCount); ++state) {
        // Reset
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                spinning[y][x] = false;
            }
        }

        // Mark which locations are spinning
        for (uint64_t fanIndex = 0; fanIndex < fanCount; ++fanIndex) {
            Point fanPosition = fans[fanIndex];
            FanDirection fanDirection = getFanDirection(fanIndex, state);

            // Note: height/width/coordinates are signed to make reverse-loops simpler
            switch (fanDirection) {
            case FanDirection::north:
                for (int y = fanPosition.y; y < height; ++y) {
                    spinning[y][fanPosition.x] = true;
                }
                break;

            case FanDirection::east:
                for (int x = fanPosition.x; x < width; ++x) {
                    spinning[fanPosition.y][x] = true;
                }
                break;

            case FanDirection::south:
                for (int y = fanPosition.y; y >= 0; --y) {
                    spinning[y][fanPosition.x] = true;
                }
                break;

            case FanDirection::west:
                for (int x = fanPosition.x; x >= 0; --x) {
                    spinning[fanPosition.y][x] = true;
                }
                break;

            default:
                abort();
            }
        }

        // Check if this is a solution (is everything spinning?)
        bool foundSolution = true;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                foundSolution &= spinning[y][x];
            }
        }

        // Print the solution
        if (foundSolution) {
            std::cout << "solution found" << std::endl;

            for (uint64_t fanIndex = 0; fanIndex < fanCount; ++fanIndex) {
                Point fanPosition = fans[fanIndex];
                FanDirection fanDirection = getFanDirection(fanIndex, state);
                std::cout << '(' << fanPosition.x << ", " << fanPosition.y << "): " << fanDirectionName(fanDirection) << std::endl;
            }

            break;
        }
    }

    return 0;
}
