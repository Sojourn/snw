#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cassert>

struct Point {
    int x;
    int y;
};

template<size_t width, size_t height>
class Board {
public:
    Board()
        : data_(0)
    {
    }

    void set(size_t x, size_t y) {
        data_ |= (1 << ((y * width) + x));
    }

    void set() {
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                set(x, y);
            }
        }
    }

    bool operator==(const Board& rhs) {
        return data_ == rhs.data_;
    }

    Board operator|(const Board& rhs) const {
        Board result;
        result.data_ = data_ | rhs.data_;
        return result;
    }

private:
    uint32_t data_;
};

template<int width, int height, int fanCount>
class Spinning {
public:
    Spinning(const Point(&fanPositions)[fanCount]) {
        for (int i = 0; i < fanCount; ++i) {
            Point fanPosition = fanPositions[i];
            {
                auto& board = boardCache_[i][0];
                for (int y = fanPosition.y; y < height; ++y) {
                    board.set(fanPosition.x, y);
                }
            }
            {
                auto& board = boardCache_[i][1];
                for (int x = fanPosition.x; x < width; ++x) {
                    board.set(x, fanPosition.y);
                }
            }
            {
                auto& board = boardCache_[i][2];
                for (int y = fanPosition.y; y >= 0; --y) {
                    board.set(fanPosition.x, y);
                }
            }
            {
                auto& board = boardCache_[i][3];
                for (int x = fanPosition.x; x >= 0; --x) {
                    board.set(x, fanPosition.y);
                }
            }
        }
    }

    const Board<width, height>& get(int fan, int state) const {
        int dir = (state >> (fan * 2)) & 3;
        return boardCache_[fan][dir];
    }

private:
    Board<width, height> boardCache_[fanCount][4];
};

int main(int argc, const char** argv) {
    Point fans[] = {
        { 0, 0 },
        { 0, 3 },
        { 2, 2 },
        { 3, 1 },
        { 3, 3 },
        { 4, 0 },
    };

    static constexpr int fanCount = sizeof(fans) / sizeof(Point);
    static constexpr int height = 4;
    static constexpr int width = 5;

    Spinning<width, height, fanCount> spinning(fans);
    Board<width, height> solution;
    solution.set();

    for (int state = 0; state < (1 << (2 * fanCount)); ++state) {
        Board<width, height> board;
        for (int fan = 0; fan < fanCount; ++fan) {
            board = board | spinning.get(fan, state);
        }
        if (board == solution) {
            std::cout << "found solution" << std::endl;
            for (int fan = 0; fan < fanCount; ++fan) {
                std::cout << '(' << fans[fan].x << ", " << fans[fan].y << "): ";
                switch ((state >> (fan * 2)) & 3) {
                case 0:
                    std::cout << "north" << std::endl;
                    break;
                case 1:
                    std::cout << "east" << std::endl;
                    break;
                case 2:
                    std::cout << "south" << std::endl;
                    break;
                case 3:
                    std::cout << "west" << std::endl;
                    break;
                }
            }

            break;
        }
    }

    return 0;
}
