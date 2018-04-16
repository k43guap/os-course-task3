//
// Lab3 tests
//

#include "../lab3.h"
#include "gtest/gtest.h"
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
std::uniform_int_distribution<int> uni(0, sleep_time*2); // guaranteed unbiased

#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds) // cross-platform sleep function
{
    auto random_integer = uni(rng);
    milliseconds += random_integer;
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}


TEST(lab3_tests, tasknumber) {
    EXPECT_GE(lab3_task_number(), 1) << "Task number must be greater than or equal to 1";
    EXPECT_LE(lab3_task_number(), 20) << "Task number must be less than or equal to 20";
}


TEST(lab3_tests, threadsync) {
    std::string line;
    std::vector<std::string> solution;
    int solution_len = 0;

    std::ostringstream filename;
    filename << "task" << lab3_task_number() << ".txt";
    std::ifstream solution_file (filename.str());
    ASSERT_TRUE( solution_file.is_open() ) << "Can't open solution file. Did you mess up with project files?";
    while ( getline (solution_file, line) )
    {
        solution.push_back(line);
        solution_len += line.length();
    }
    solution_file.close();
//    std::cout << solution_len << std::endl;

//    std::cout << "Running tests for lab3 ..." << std::endl;
    std::stringstream outs;
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(outs.rdbuf()); //redirect std::cout to out.txt!
    // call student's lab3 code
    int n = lab3_init();
    std::cout.rdbuf(coutbuf); //reset to standard output again
    EXPECT_EQ(n, 0) << "If 'lab3_init()' succeeds, it should return 0.";
    EXPECT_GE(outs.str().length(), solution_len*3) << "Output is too short.";
    EXPECT_LE(outs.str().length(), solution_len*5) << "Output is too long.";
    //
    std::cout << "Task " << lab3_task_number() << ": " << outs.str() << std::endl;
//    std::cout << outs.str().length() << std::endl;
//    std::cout << solution.size() << std::endl;
    int interval = 0;
    std::map <char, int> iterval_stats;
    char c_cur = 0, c_prev = 0;
    bool is_parallel = (solution.at(interval).length() == 1);
    for (int i = 0; i < outs.str().length(); ++i) {
        //
        if (isspace(outs.str().at(i)))
            continue;
        c_prev = c_cur;
        c_cur = outs.str().at(i);
//        std::cout << i << ", " << std::flush;
//        std::cout << c_cur <<", " << c_prev << ", " << interval << ", " << solution.at(interval) << ", " << is_parallel << std::endl;
        if (solution.at(interval).find(c_cur) != std::string::npos) {
            if ((c_cur != c_prev) && (iterval_stats[c_cur] > 0))
                is_parallel = true;
            iterval_stats[c_cur] += 1;
            continue;
        }
        else if (interval+1 < solution.size() && solution.at(interval + 1).find(c_cur) != std::string::npos) {
            // check that the interval was "good"
            EXPECT_TRUE(is_parallel) << "It seems that your threads are not running in parallel. "
                                     << "Check the '" << solution.at(interval) << "' interval";
            for (int j = 0; j < solution.at(interval).length(); ++j) {
                EXPECT_GT(
                        iterval_stats[solution.at(interval).at(j)],
                        0
                ) << "Thread '" << solution.at(interval).at(j)
                  << "' did not run during the '" << solution.at(interval)
                  << "' interval";
            }
            // all tests passed, can go to the next interval now
            ++interval;
            iterval_stats.clear();
            for (int j = 0; j < solution.at(interval).length(); ++j)
                iterval_stats[solution.at(interval).at(j)] = 0;
            iterval_stats[c_cur] += 1;
            // if we might have split the intervals inaccurately, give 'c_prev' thread more credit to justify
            if (solution.at(interval).find(c_prev) != std::string::npos)
                iterval_stats[c_prev] += 1;
            is_parallel = (solution.at(interval).length() == 1);
        }
        else {
            std::ostringstream msg;
            msg << "Unexpected character '" << c_cur << "' was found while looking for any of '"
                << solution.at(interval) << "'";
            if (interval+1 < solution.size())
                    msg << " or '" << solution.at(interval+1) << "'";
            msg << ".";
            ASSERT_TRUE(false) << msg.str();
        }
    }
    EXPECT_TRUE(is_parallel) << "It seems that your threads are not running in parallel. "
                             << "Check the '" << solution.at(interval) << "' interval";
    for (int j = 0; j < solution.at(interval).length(); ++j){
        EXPECT_GT(
                iterval_stats[solution.at(interval).at(j)],
                0
        ) << "Thread '" << solution.at(interval).at(j)
          << "' did not run during the '" << solution.at(interval)
          << "' interval";
    }
    // check that all intervals were found in the data
    EXPECT_EQ(interval, solution.size()-1) << "Some intervals are missing. "
                                           << "Are you sure you didn't forget to start any threads?";
}

TEST(lab3_tests, concurrency) {
    std::stringstream outs;
    std::string outs_prev;
    bool is_random = false;
    int n = 0;
    for (int i = 0; i < 100; ++i) {
        outs_prev = outs.str();
        outs.str( std::string() );
        outs.clear();
        std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
        std::cout.rdbuf(outs.rdbuf()); //redirect std::cout to out.txt!
        // call student's lab3 code
        n = lab3_init();
        std::cout.rdbuf(coutbuf); //reset to standard output again

//        std::cout << outs.str() << " (" << outs.str().length() << ")" << std::endl;
        EXPECT_EQ(n, 0) << "If 'lab3_init()' succeeds, it should return 0. Did it fail?";
        if (i > 0) {
            EXPECT_EQ(outs.str().length(), outs_prev.length()) << "Concurrent execution of threads "
                                                               << "must not produce variable length "
                                                               << "results.";
        }
        if (outs_prev.compare(outs.str()) != 0)
            is_random = true;
    }
    EXPECT_TRUE(is_random) << "It seems that there is no real concurrency going on. "
                           << "Either fix the code or try the test again.";
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
