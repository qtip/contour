#include "contour.h"
#include <iomanip>

bool float_lt(float a, float b, float epsilon) {
    return fabs(a-b) > epsilon && a < b;
}
bool float_lt(float a, float b) {
    return float_lt(a, b, EPSILON);
}
bool float_eq(float a, float b, float epsilon) {
    return fabs(a-b) < epsilon;
}
bool float_eq(float a, float b) {
    return float_eq(a, b, EPSILON);
}

bool Point::operator<(const Point &b) const {
    if (*this == b) {
        return false;
    }
    return float_lt(x,  b.x) || (!float_lt(b.x, x) && float_lt(y, b.y));
}
bool Point::operator==(const Point &b) const {
    return float_eq(x, b.x) && float_eq(y, b.y);
}
bool Point::eq(const Point &b, float epsilon) const {
    return float_eq(x, b.x, epsilon) && float_eq(y, b.y, epsilon);
}

std::ostream &operator<<(std::ostream &out, const Point &point) {
    out<<"("<<point.x<<","<<point.y<<")";
    return out;
}

void PathBuilder::add(Point a, Point b) {
    using namespace std;
    auto backs_contains_a = backs.find(a) != backs.end();
    auto backs_contains_b = backs.find(b) != backs.end();
    auto fronts_contains_a = fronts.find(a) != fronts.end();
    auto fronts_contains_b = fronts.find(b) != fronts.end();

    if (fronts_contains_a || backs_contains_b) {
        swap(a, b);
        backs_contains_a = backs.find(a) != backs.end();
        backs_contains_b = backs.find(b) != backs.end();
        fronts_contains_a = fronts.find(a) != fronts.end();
        fronts_contains_b = fronts.find(b) != fronts.end();
    }

    if (a == b) {
        return;
    }

    if (!fronts_contains_b && !backs_contains_a && !fronts_contains_a && !fronts_contains_b) {
        // add a new path
        paths.emplace_back();
        paths.back().push_back(a);
        paths.back().push_back(b);
        fronts[a] = paths.size()-1;
        backs[b] = paths.size()-1;
    } else if (fronts_contains_b && backs_contains_a && (fronts[b] == backs[a]) && !backs_contains_b && !fronts_contains_a) {
        // close a loop
        paths[backs[a]].push_back(b);

        backs.erase(a);
        fronts.erase(b);
    } else if (backs_contains_b && backs_contains_a && !fronts_contains_a && !fronts_contains_b) {
        // join two paths
        auto front_path_index = backs[a];
        auto back_path_index = backs[b];
        auto &front_path = paths[front_path_index];
        auto &back_path = paths[back_path_index];

        fronts.erase(back_path.front());
        backs[back_path.front()] = front_path_index;
        backs.erase(a);
        backs.erase(b);

        front_path.insert(front_path.end(), back_path.rbegin(), back_path.rend());
        back_path.clear();
        back_path.shrink_to_fit();
    } else if (fronts_contains_b && fronts_contains_a && !backs_contains_a && !backs_contains_b) {
        // join two paths
        auto front_path_index = fronts[a];
        auto back_path_index = fronts[b];
        auto &front_path = paths[front_path_index];
        auto &back_path = paths[back_path_index];

        backs.erase(back_path.back());
        fronts[back_path.back()] = front_path_index;
        fronts.erase(a);
        fronts.erase(b);

        front_path.insert(front_path.begin(), back_path.rbegin(), back_path.rend());
        back_path.clear();
        back_path.shrink_to_fit();
    } else if (fronts_contains_b && backs_contains_a && !fronts_contains_a && !backs_contains_b) {
        // join two paths
        auto front_path_index = backs[a];
        auto back_path_index = fronts[b];
        auto &front_path = paths[front_path_index];
        auto &back_path = paths[back_path_index];

        backs[back_path.back()] = front_path_index;
        fronts.erase(a);
        fronts.erase(b);

        front_path.insert(front_path.end(), back_path.begin(), back_path.end());
        back_path.clear();
        back_path.shrink_to_fit();
    } else if (fronts_contains_b && !fronts_contains_a && !backs_contains_b && !backs_contains_a) {
        // prepend to an existing path
        paths[fronts[b]].push_front(a);
        fronts[a] = fronts[b];
        fronts.erase(b);
    } else if (backs_contains_a && !fronts_contains_b && !fronts_contains_a && !backs_contains_b) {
        // append to an existing path
        paths[backs[a]].push_back(b);
        backs[b] = backs[a];
        backs.erase(a);
    } else {
        if (fronts_contains_a) {
            cerr<<"fronts contains a\n";
            cerr<<"    fronts["<<a<<"] == "<<fronts[a]<<"\n";
        }
        if (fronts_contains_b) {
            cerr<<"fronts contains b\n";
            cerr<<"    fronts["<<b<<"] == "<<fronts[b]<<"\n";
        }
        if (backs_contains_a) {
            cerr<<"backs contains a\n";
            cerr<<"    backs["<<a<<"] == "<<backs[a]<<"\n";
        }
        if (backs_contains_b) {
            cerr<<"backs contains b\n";
            cerr<<"    backs["<<b<<"] == "<<backs[b]<<"\n";
        }
        cout<<"<line x1=\""<<a.x<<"\" y1=\""<<a.y<<"\" x2=\""<<b.x<<"\" y2=\""<<b.y<<"\" style=\"stroke:rgb(0,0,255);stroke-width:1.30\" />";
        cout<<"<line x1=\""<<a.x-10<<"\" y1=\""<<a.y-10<<"\" x2=\""<<a.x-20<<"\" y2=\""<<a.y-20<<"\" style=\"stroke:rgb(0,0,255);stroke-width:1.00\" />";
        exit(32);
    }
}
void PathBuilder::write_out(std::ostream &out) const {
    for (const auto &path : paths) {
        if (path.empty()) {
            continue;
        }
        out << std::setprecision(4);
        out<<"<path d=\"M";
        int count = 0;
        for (const auto &point : path) {
            if (count == 1) {
                out<<"L";
            }
            out<<point.x<<" "<<point.y<<" ";
            count++;
        }
        if (path.front() == path.back()) {
            out<<" Z";
        }
        out<<"\" />\n";
    }
}

std::ostream &operator<<(std::ostream &out, const PathBuilder &pb) {
    pb.write_out(out);
    return out;
}
