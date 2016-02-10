#ifndef CONTOUR_H
#define CONTOUR_H
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <map>
#include <cstring>
#include <cmath>
#include <utility>
#include <limits>
#include <algorithm>


struct Line {
    float x1, y1, x2, y2;
};

const float EPSILON = 0.00000001f;
const char NORTH = 0b0001;
const char EAST = 0b0010;
const char SOUTH = 0b0100;
const char WEST = 0b1000;
const char cases[] = {
    0,
    SOUTH | WEST,
    SOUTH | EAST,
    EAST | WEST,
    NORTH | EAST,
    NORTH | EAST | SOUTH | WEST,
    NORTH | SOUTH,
    NORTH | WEST,
    NORTH | WEST,
    NORTH | SOUTH,
    NORTH | EAST | SOUTH | WEST,
    NORTH | EAST,
    EAST | WEST,
    SOUTH | EAST,
    SOUTH | WEST,
    0
};

struct Point {
    float x, y;
    bool operator<(const Point &b) const;
    bool operator==(const Point &b) const;
    bool eq(const Point &b, float epsilon) const;
};

class PathBuilder {
public:

    void add(Point a, Point b);
    void write_out(std::ostream &out) const;
private:
    std::vector<std::deque<Point>> paths;
    std::map<Point, int> fronts;
    std::map<Point, int> backs;
};


template<class I>
void work(I source,
          const unsigned long long width,
          const unsigned long long height,
          const float step,
          const float offset,
          std::map<typename std::remove_reference<decltype(*source)>::type, PathBuilder> &path_builders,
          std::mutex &read_mutex,
          std::mutex &write_mutex,
          unsigned long long &line,
          std::vector<typename std::remove_reference<decltype(*source)>::type> &buffer) {
    using namespace::std;
    typedef typename remove_reference<decltype(*source)>::type value_t;
    unsigned long long local_line;

    vector<value_t> buffer1;
    buffer1.resize(width);

    vector<value_t> buffer2;
    buffer2.resize(width);

    map<value_t, vector<Line>> lines;

    while(true) {
        // get next two rows of data to work on
        {
            lock_guard<mutex> lock(read_mutex);
            if (line+1 >= height) {
                break;
            }
            // copy current line buffer to local buffer
            copy(buffer.begin(), buffer.end(), buffer1.begin());
            // read in next line from file into global buffer
            auto from = source + (line+1) * width;
            auto to = from + width;
            copy(from, to, buffer.begin());
            //read_stream.read(reinterpret_cast<char*>(buffer.data()), row_bytes);
            // copy current line buffer to local buffer
            copy(buffer.begin(), buffer.end(), buffer2.begin());
            local_line = line++;
        }
        for (int i = 0; i < width-1; i++) {
            float sw = buffer2[i+0];
            float se = buffer2[i+1];
            float ne = buffer1[i+1];
            float nw = buffer1[i+0];
            float lowest_value = min(sw, min(se, min(ne, nw)));
            float highest_value = max(sw, max(se, max(ne, nw)));
            float lowest_step = ceil((lowest_value-offset)/step)*step + offset;

            for (float elevation = lowest_step; elevation <= highest_value; elevation += step) {
                int cell = (sw > elevation ? 1 : 0)
                           + (se > elevation ? 2 : 0)
                           + (ne > elevation ? 4 : 0)
                           + (nw > elevation ? 8 : 0);
                if (cell != 0x0 && cell != 0xf ) {
                    Point points[4];
                    int point_idx = 0;
                    int sides = cases[cell];
                    if (sides & NORTH) {
                        float a = nw;
                        float b = ne;
                        points[point_idx].x = (elevation-a)/(b-a);
                        points[point_idx].y = 0;
                        point_idx++;
                    }
                    if (sides & EAST) {
                        float a = ne;
                        float b = se;
                        points[point_idx].x = 1;
                        points[point_idx].y = (elevation-a)/(b-a);
                        point_idx++;
                    }
                    if (sides & SOUTH) {
                        float a = sw;
                        float b = se;
                        points[point_idx].x = (elevation-a)/(b-a);
                        points[point_idx].y = 1;
                        point_idx++;
                    }
                    if (sides & WEST) {
                        float a = nw;
                        float b = sw;
                        points[point_idx].x = 0;
                        points[point_idx].y = (elevation-a)/(b-a);
                        point_idx++;
                    }
                    Line line = Line{points[0].x + i, points[0].y + local_line, points[1].x + i, points[1].y + local_line};
                    if (lines.find(elevation) == lines.end()) {
                        lines[elevation] = vector<Line>();
                    }
                    lines[elevation].push_back(line);
                    if (point_idx == 4) {
                        Line line = Line{points[2].x + i, points[2].y + local_line, points[3].x + i, points[3].y + local_line};
                        lines[elevation].push_back(line);
                    }
                }
            }
        }
        {
            lock_guard<mutex> lock(write_mutex);
            for (const auto &pair : lines) {
                for (const auto &line : pair.second) {
                    path_builders[pair.first].add(Point{line.x1, line.y1}, Point{line.x2, line.y2});
                }
            }
            lines.clear();
        }
    }
}

template<class I>
auto generate(
    I source,
    const unsigned long long width,
    const unsigned long long height,
    const float step,
    const float offset) {
    using namespace::std;
    typedef typename remove_reference<decltype(*source)>::type value_t;

    vector<thread> threads;
    mutex read_mutex, write_mutex;
    vector<value_t> buffer;
    buffer.resize(width);

    copy(source, source + width, buffer.begin());

    map<value_t, PathBuilder> output;
    unsigned long long line = 0;

    for (int i = 0; i < thread::hardware_concurrency(); ++i) {
        threads.emplace_back(work<I>,
                             source,
                             width,
                             height,
                             step,
                             offset,
                             ref(output),
                             ref(read_mutex),
                             ref(write_mutex),
                             ref(line),
                             ref(buffer));
    }
    for (int i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
    return output;
}

#endif /* CONTOUR_H */
