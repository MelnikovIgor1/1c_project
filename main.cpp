#include <iostream>
#include <vector>
#include <set>
#include <map>

#include <queue>
#include <list>
#include <fstream>
#include <regex>

#include "json.hpp"
using json = nlohmann::json;

class Course {
public:
    std::vector<Course*> required_courses;
    int level;
    std::string title;
    int num;

    template<typename T>
    static Course* Make_course(const T& data);

    void Set_requirements(std::vector<Course*>& nums) {
        for (auto index: raw_required_) {
            required_courses.push_back(nums[index]);
        }
    }

private:
    // indexes of required courses
    std::vector<int> raw_required_;

};

template<typename T>
Course* Course::Make_course(const T& data) {
    auto* node = new Course;

    node->level = data["level"];
    node->num = data["num"];
    node->title = data["title"].template get<std::string>();
    auto str = data["requirments"].template get<std::string>();


    int x;

    std::regex regex(R"(\d+)");
    std::smatch match;
    while (std::regex_search(str, match, regex)) {
        node->raw_required_.push_back(std::stoi(match.str()));
        str = match.suffix();
    }

    return node;
}

struct Program {
    /*
    Program is set of courses for each level
     all courses in program are guaranteed to have all
     required courses in the same program
     */
    Program() : courses(5) {

    }

    int stage = 0;
    /*
     1 - has 5-level course
     2-4 has at least stage-1 3-level courses
     */

    std::vector<std::set<Course*>> courses;

    /*
     adds course and all pre-prerequirements
     */
    void Add_course(Course* course) {
        std::queue<Course*> queue;
        queue.push(course);

        while (!queue.empty()) {
            auto new_course = queue.front();
            queue.pop();

            for (auto child: new_course->required_courses) {
                queue.push(child);
            }

            courses[new_course->level - 1].insert(new_course);
        }
    }

    int Get_courses_num(int level) {
        return courses[level - 1].size();
    }

    bool Have_course(Course* course) {
        return courses[course->level - 1].count(course) > 0;
    }
};

class Schedule {
public:
    /*
    Schedule has is timetable for students -
     it has it's program (all courses in schedule)
     and have to put them in semesters
     */
    Schedule(Program program) :
        program_(std::move(program)),
        semesters(semesters_num_) {
        make_timetable();
    }
    Schedule() :
            program_(),
            semesters(semesters_num_){
    }

    void make_timetable() {
        for (auto& level: program_.courses) {
            for (auto course: level) {
                Put(course);
            }
        }
    }

    void Put(Course* node) {
        auto from = Get_min_semester(node);

        bool done = false;
        for (int i = from; i < semesters.size(); ++i) {
            if (semesters[i].size() >= min_courses_) {
                continue;
            }

            Insert_course_(node, i);
            done = true;
        }

        if (!done) {
            Insert_course_(node, 0);
        }
    }

private:
    void Insert_course_(Course* node, int semester) {
        semesters[semester].insert(node);
        course_semester_map_.insert({node, semester});
    }

    int Get_min_semester(Course* node) {
        int answer = 0;
        for (auto child: node->required_courses) {
            answer = std::max(answer, course_semester_map_[child] + 1);
        }

        return answer;
    }

    std::vector<std::set<Course*>> semesters;
    Program program_;
    std::map<Course*, int> course_semester_map_;
    static const int semesters_num_ = 7;
    static const int min_courses_ = 3;
};

void Make_schedule() {
    std::vector<std::vector<Course*>> all_courses;
    std::queue<Program> raw_tracks;
    std::queue<Schedule> tracks;

    for (auto level5: all_courses[4]) {
        raw_tracks.emplace();
        raw_tracks.back().Add_course(level5);
        raw_tracks.back().stage = 1;
    }

    while (!raw_tracks.empty()) {
        auto cur = std::move(raw_tracks.front());
        raw_tracks.pop();

        if (cur.stage < 4) {
            auto level3_cnt = cur.Get_courses_num(3);
            if (level3_cnt >= 3) {
                cur.stage = 4;
                raw_tracks.push(std::move(cur));
                continue;
            }

            cur.stage = level3_cnt + 1;

            for (auto course3: all_courses[3]) {
                if (cur.Have_course(course3)) {
                    continue;
                }

                auto copy = cur;
                copy.Add_course(course3);
                copy.stage = copy.Get_courses_num(3); + 1;
                raw_tracks.push(copy);
            }
        } else {
            tracks.emplace(cur);
        }
    }
}

std::vector<Course*> Prepare_courses(const std::string& json_file) {
    std::vector<Course*> nodes;
    std::vector<Course*> nums;
    std::ifstream ifs(json_file);
    std::stringstream buf; buf << ifs.rdbuf();

    json jf = json::parse(buf.str());

    for (auto& item: jf) {
        nodes.push_back(Course::Make_course(item));
        int n = nodes.back()->num;
        if (nums.size() < n + 1) {
            nums.resize(n + 1);
        }
        nums[n] = nodes.back();
    }

    for (auto& node: nodes) {
        node->Set_requirements(nums);
    }

    return nodes;
}

int main() {
    std::vector<Course*> nodes = Prepare_courses("data.txt");

    for (auto node: nodes) {
        delete node;
    }

    return 0;
}
