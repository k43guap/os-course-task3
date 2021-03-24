//
// Lab3 tests
//

#include "../lab3.h"
#include "gtest/gtest.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
// std::uniform_int_distribution<int> uni(0, sleep_time*2); // guaranteed unbiased
std::uniform_int_distribution<int> uni_delay(10, 100); // guaranteed unbiased
std::uniform_int_distribution<int> uni_chance(0, 100); // guaranteed unbiased

#if defined WIN32 || defined _WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds) // cross-platform sleep function
{
    // auto random_integer = uni(rng);
    // milliseconds += random_integer;
#if defined WIN32 || defined _WIN32
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


void computation()
{
    auto delay = uni_delay(rng);
    auto chance = uni_chance(rng);
    sleep_ms(delay * sleep_time);
    if (chance < 10) {
        sleep_ms(delay);
    }
}




std::string get_unique_chars(std::string str)
{
    std::sort(std::begin(str), std::end(str));
    auto last = std::unique(std::begin(str), std::end(str));
    str.erase(last, std::end(str));
    return str;
}


std::vector< std::string > split_intervals(const char * intervals, char sep = ',')
{
    std::stringstream ss( intervals );
    std::vector< std::string > thread_intervals;
    while( ss.good() )
    {
        std::string substr;
        getline( ss, substr, sep );
        thread_intervals.push_back( substr );
    }
    return thread_intervals;
}


std::vector< std::string > load_solution( int &solution_len )
{
    std::string line;
    std::vector< std::string > solution;
    // int solution_len = 0;
    solution_len = 0;

    std::ostringstream filename;
    filename << "task" <<  lab3_thread_graph_id() << ".txt";
    std::ifstream solution_file (filename.str());
    // ASSERT_TRUE( solution_file.is_open() ) << "Can't open solution file. Did you mess up with project files?";
    // TODO: make this assert in a toplevel TEST by checking the solution_len variable
    if (!solution_file.is_open())
        solution_len = -1;
    else {
        while ( getline (solution_file, line) ) {
            solution.push_back(line);
            solution_len += line.length();
        }
        solution_file.close();
    }
    return solution;
}


static bool map_compare(const std::pair<char,int> & p1, const std::pair<char,int> & p2)
{
    return p1.second < p2.second;
}

int get_thread_length(std::string str, std::vector< std::string > solution)
{
    size_t interval = 0;
    int interval_length = -1;
    std::map <char, int> interval_stats;
    char c_cur = 0, c_prev = 0;
    // bool is_parallel = (solution.at(interval).length() == 1);
    for (size_t i = 0; i < str.length(); ++i) {
        //
        if (isspace(str.at(i)))
            continue;
        c_prev = c_cur;
        c_cur = str.at(i);
//        std::cout << i << ", " << std::flush;
//        std::cout << c_cur <<", " << c_prev << ", " << interval << ", " << solution.at(interval) << ", " << is_parallel << std::endl;
        if (solution.at(interval).find(c_cur) != std::string::npos) {
            interval_stats[c_cur] += 1;
            continue;
        }
        else if (interval+1 < solution.size() && solution.at(interval + 1).find(c_cur) != std::string::npos) {
            // all tests passed, can go to the next interval now
            ++interval;
            if (interval_length <= 0) {
                // see https://stackoverflow.com/questions/9370945/c-help-finding-the-max-value-in-a-map
                // interval_length = *std::min_element(interval_stats.begin(), interval_stats.end(), [] (const std::pair<char,int> & p1, const std::pair<char,int> & p2) {
                //     return p1.second < p2.second;
                // });
                interval_length = std::min_element(interval_stats.begin(), interval_stats.end(), map_compare)->second;
            }
            return interval_length;
        }
    }
    return -1;
}


bool is_space_char(char c)
{
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}


std::vector< std::string > get_interval_contents(std::string output_str, std::vector< std::string > solution)
{
    // split source output string into intervals according to solution info
    std::vector< std::string > interval_contents;
    // output_str = outs.str();
    // remove all whitespaces from the string
    // https://stackoverflow.com/questions/31959532/best-way-to-remove-white-spaces-from-stdstring
    output_str.erase(std::remove_if(
        output_str.begin(), output_str.end(),
        // [l = std::locale{}](auto ch) { return std::isspace(ch, l); }
        is_space_char
    ), output_str.end());
    // find out the length of a thread
    int thread_length = get_thread_length(output_str, solution);
    size_t pos = 0;
    if (thread_length > 0) {
        // iterate through solution intervals and cut a corresponding portion of output
        for (size_t i = 0; i < solution.size(); ++i) {
            //
            int interval_length = solution.at(i).length() * thread_length;
            interval_contents.push_back(output_str.substr(pos, interval_length));
            pos += interval_length;
        }
        if (pos < output_str.length())
            interval_contents.push_back(output_str.substr(pos, std::string::npos));
    }
    return interval_contents;
}


bool is_permutation_str(std::string s1, std::string s2)
{
    // make sure this runs with C++11 only;
    // passing end() for the second iterator as fourth argument to std::is_permutation 
    // is supported starting from c++14 only
    if (s1.length() > s2.length())
        return std::is_permutation(s1.begin(), s1.end(), s2.begin());
    else
        return std::is_permutation(s2.begin(), s2.end(), s1.begin());
}


std::string check_sequential(std::string output_str, std::vector< std::string > solution)
{
    // std::string output_str = outs.str();
    // load intervals that are expected to be sequential
    std::vector< std::string > intervals = split_intervals(lab3_sequential_threads());
    // iterate through all sequential intervals and search for each of them in the output
    std::vector< std::string > results = get_interval_contents(output_str, solution);
    for (size_t i = 0; i < intervals.size(); ++i) {
        std::string s1 = intervals.at(i);
        bool interval_found = false;
        for (size_t j = 0; j < results.size(); ++j) {
            std::string s2 = get_unique_chars(results.at(j));
            // if (std::is_permutation(s1.begin(), s1.end(), s2.begin(), s2.end())) {
            if (is_permutation_str(s1, s2)) {
                interval_found = true;
                // check if the same pattern is repeated several times
                std::string pattern = results.at(j).substr(0, s1.length());
                size_t pos = 0, pos_prev = std::string::npos;
                while (pos != std::string::npos) {
                    pos_prev = pos;
                    pos = results.at(j).find(pattern, pos+1);
                    if ((pos == std::string::npos && pos_prev + s1.length() != results.at(j).length()) 
                        || (pos != std::string::npos && pos != pos_prev + s1.length())) {
                        // extra characters found, report error
                        std::stringstream ss;
                        ss << "Threads in interval " << results.at(j) 
                            << " are not sequential! Expected repetition of " << pattern 
                            << " not found at position " << pos_prev + s1.length() 
                            << ". Instead character \'" << results.at(j).at(pos_prev + s1.length()) 
                            << "\' was found. Full output is " << output_str;
                        return ss.str();
                    }
                }
            }
        }
        if (!interval_found) { // interval was not found in the output
            std::stringstream ss;
            ss << "Interval with sequential threads " << s1 << " not found in " << output_str;
            return ss.str();
        }
    }
    return "";
}

std::string check_unsynchronized(std::string output_str, std::vector< std::string > solution)
{
    // std::string output_str = outs.str();
    // load intervals that are expected to be sequential
    std::vector< std::string > intervals = split_intervals(lab3_unsynchronized_threads());
    // iterate through all unsynchronized intervals and search for each of them in the output
    std::vector< std::string > results = get_interval_contents(output_str, solution);
    for (size_t i = 0; i < intervals.size(); ++i) {
        std::string s1 = intervals.at(i);
        bool interval_found = false;
        for (size_t j = 0; j < results.size(); ++j) {
            std::string s2 = get_unique_chars(results.at(j));
            // if (std::is_permutation(s1.begin(), s1.end(), s2.begin(), s2.end())) {
            if (is_permutation_str(s1, s2)) {
                interval_found = true;
                size_t pos = 0, step = s1.length();
                std::string sequence, prev_sequence;
                bool is_unsync = false;
                // check that threads on the interval are unsynchronized
                while (!is_unsync && pos < results.at(j).length()) {
                    prev_sequence = sequence;
                    sequence = results.at(j).substr(pos, step);
                    pos += step;
                    if (sequence != prev_sequence)
                        is_unsync = true;
                }
                if (!is_unsync) {
                    // threads are synchronized, but they shouldn't be
                    std::stringstream ss;
                    ss << "Threads in interval " << results.at(j)
                        << " seem to be synchronized, while they are expected not to be in sync!";
                    return ss.str();
                }
            }
        }
        if (!interval_found) { // interval was not found in the output
            std::stringstream ss;
            ss << "Interval with unsynchronized threads " << s1 << " not found. None of the following intervals matched:" << std::endl;
            for (std::vector< std::string >::iterator iter = results.begin(); iter != results.end(); ++iter)
                ss << *iter << std::endl;
            return ss.str();
        }
    }
    return "";
}


TEST(lab3_tests, tasknumber) {
    EXPECT_GE(lab3_thread_graph_id(), 1)  << "Thread graph must be greater than or equal to 1";
    EXPECT_LE(lab3_thread_graph_id(), 20) << "Thread graph must be less than or equal to 20";
    std::cout << "TASKID is " << lab3_thread_graph_id() << std::endl;
}


TEST(lab3_tests, unsynchronizedthreads) {
    // string s = lab3_unsynchronized_threads();
    std::vector< std::string > intervals = split_intervals(lab3_unsynchronized_threads());
    for (std::vector< std::string >::iterator t=intervals.begin(); t!=intervals.end(); ++t)
    {
        EXPECT_GE(t->length(), 3) << "At least 3 threads must be running unsynchronized, but only " << t->length() << " were given (" << *t << ")";
        EXPECT_LE(t->length(), 7) << "Too many unsynchronized threads per interval! Did you forget to separate threads from different intervals with comma?";
    }
    std::cout << "Unsynchronized threads are " << lab3_unsynchronized_threads() << std::endl;
}


TEST(lab3_tests, sequentialthreads) {
    // string s = lab3_unsynchronized_threads();
    std::vector< std::string > intervals = split_intervals(lab3_sequential_threads());
    for (std::vector< std::string >::iterator t=intervals.begin(); t!=intervals.end(); ++t)
    {
        EXPECT_GE(t->length(), 3) << "At least 3 threads must be running sequentially, but only " << t->length() << " were given (" << *t << ")";
        EXPECT_LE(t->length(), 7) << "Too many sequential threads per interval! Did you forget to separate threads from different intervals with comma?";
    }
    std::cout << "Sequential threads are " << lab3_sequential_threads() << std::endl;
}


TEST(lab3_tests, threadsync) {
    std::vector< std::string > solution;
    int solution_len = 0;
    solution = load_solution(solution_len);
    ASSERT_NE( solution_len, -1 ) << "Can't open solution file. Did you mess up with project files?";
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
    std::cout << "Output for graph " << lab3_thread_graph_id() << " is: " << outs.str() << std::endl;
//    std::cout << outs.str().length() << std::endl;
//    std::cout << solution.size() << std::endl;
    std::vector< std::string > results = get_interval_contents(outs.str(), solution);
    std::stringstream ss;
    ss << "Intervals are:" << std::endl;
    for (std::vector< std::string >::iterator iter = results.begin(); iter != results.end(); ++iter)
        ss << *iter << std::endl;
    std::cout << ss.str();
        
    EXPECT_EQ(results.size(), solution.size()) << "Invalid number of intervals. Expected " << solution.size() << " intervals, but found " << results.size() << ". Did you forget to run some threads? Otherwise maybe your threads are running for too long (or too short)?";
    std::map <char, int> interval_stats;
    bool is_parallel = false;
    for (size_t i = 0; i < results.size(); ++i) {
        // if i-th interval consists of just one thread, do not check for parallelism
        is_parallel = (solution.at(i).length() == 1);
        for (size_t j = 0; j < results.at(i).length(); ++j) {
            // check if this thread is running not for the first time during current interval
            if (j > 0 && results.at(i).at(j) != results.at(i).at(j-1) && interval_stats[results.at(i).at(j)] > 0)
                is_parallel = true;
            // update character counter
            interval_stats[results.at(i).at(j)] += 1;
            // check that current character is present in solution
            if (solution.at(i).find( results.at(i).at(j) ) == std::string::npos)
                ASSERT_TRUE(false) << "Unexpected character '" << results.at(i).at(j) << "' was found while looking for any of '"
                    << solution.at(i) << "'.";
        }
        EXPECT_TRUE(is_parallel) << "It seems that your threads are not running in parallel. "
                                 << "Check the '" << solution.at(i) << "' interval";
        for (size_t j = 0; j < solution.at(i).length(); ++j) {
            EXPECT_GT(
                    interval_stats[solution.at(i).at(j)],
                    0
            ) << "Thread '" << solution.at(i).at(j)
              << "' did not run during the '" << solution.at(i)
              << "' interval";
        }
    }
}


TEST(lab3_tests, concurrency) {
    std::stringstream outs;
    std::string outs_prev;
    std::vector< std::string > solution, err_messages;
    int solution_len = 0;
    solution = load_solution(solution_len);
    ASSERT_NE( solution_len, -1 ) << "Can't open solution file. Did you mess up with project files?";
    
    bool is_random = false, sequential_failed = false, unsynchronized_failed = false;
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
                                                               << "results. Current output: " << outs.str()
                                                               << "Previous output: " << outs_prev;
        }
        if (outs_prev.compare(outs.str()) != 0)
            is_random = true;
        std::string output_str = outs.str();
        std::stringstream err_ss;
        std::string sequential_err = check_sequential(output_str, solution);
        err_ss << "Execution #" << i << ". Output is: " << outs.str();
        if (sequential_err.length() > 0) {
            err_ss << std::endl << "Sequential execution test failed!" << std::endl << sequential_err << std::endl;
            sequential_failed = true;
        }
        // EXPECT_EQ(sequential_err.length(), 0) << sequential_err << std::endl << "Output is: " << outs.str();
        std::string unsynchronized_err = check_unsynchronized(output_str, solution);
        if (unsynchronized_err.length() > 0) {
            err_ss << std::endl << "Unsynchronized execution test failed!" << std::endl << unsynchronized_err << std::endl;
            unsynchronized_failed = true;
        }
        // EXPECT_EQ(unsynchronized_err.length(), 0) << unsynchronized_err << std::endl << "Output is: " << outs.str();
        if (sequential_err.length() > 0 || unsynchronized_err.length() > 0)
            err_messages.push_back(err_ss.str());
        // be verbose about what is going on
        if (i % 20 == 0)
            std::cout << "Completed " << i << " out of 100 runs." << std::endl;
    }
    EXPECT_TRUE(is_random) << "It seems that there is no real concurrency going on. "
                           << "Either fix the code or try the test again.";
    if (err_messages.size() > 0) {
        EXPECT_FALSE(sequential_failed) << "Sequential execution test(s) failed!";
        EXPECT_FALSE(unsynchronized_failed) << "Unsynchronized execution test(s) failed!";
        int msg_count = fmin(5, err_messages.size());
        for (int i = 0; i < msg_count; ++i)
            std::cerr << err_messages.at(i) << std::endl;
        if (err_messages.size() > 5) {
            std::cerr << "Too many error messages. Error log trimmed." << std::endl;
            std::ofstream err_log("test.log");
            for (std::vector< std::string >::iterator msg = err_messages.begin(); msg != err_messages.end(); ++msg)
                err_log << *msg << std::endl;
            err_log.close();
            std::cerr << "Full log dumped to file test.log" << std::endl;
        }
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

