#ifndef __MYLIBS_TRACE__
#define __MYLIBS_TRACE__

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#define RESET "\033[0m"
#define RED "\033[31m"    /* Red */
#define GREEN "\033[32m"   /* Green */
#define YELLOW "\033[33m"   /* Yellow */

class Trace;

extern std::vector<std::string> trace_msg_recordor;
extern std::vector<Trace*> trace_recordor;

class Extra_File
{
public:
    static std::vector<std::string> read_lines(const char* _file, int _line_num,
                                 int _up_offset=0, int _down_offset=0) {
        std::vector<std::string> _strs;
        std::ifstream in;
        in.open(_file);
        int line = 1;
        char data[1024];
        while (in.getline(data, 1024))
        {
            if ((_line_num-_up_offset) == line)
            {
                _strs.push_back(data);
                break;
            }
            line ++ ;
        }
        for(int i = 0; i < (_up_offset+_down_offset+1); i++) {
            in.getline(data, 1024);
            _strs.push_back(data);
        }
        in.close();
        return _strs;
    }
};

class Trace
{
public:
    Trace() { _is_empty = true; }
    Trace(const char* _file, const char* _func, int _line, int offset=0) {
        std::ostringstream s;
        s << "File '" << YELLOW << _file << RESET
          << "', line " << YELLOW << _line+offset << RESET
          << ", in " << GREEN << _func << RESET
          << ":" << std::endl;
        msg = s.str();
        _fl = _file;
        _l = _line+offset;

        trace_recordor.push_back(this);
        trace_msg_recordor.push_back(msg);
        _is_empty = false;
        _recorder_id = trace_msg_recordor.size() - 1;
    }
    Trace(const Trace& t) {
        msg = t.msg;
        _fl = t._fl;
        _l = t._l;
        _is_empty = _is_empty;
        _recorder_id = t._recorder_id;
        trace_recordor[_recorder_id] = this;
        trace_msg_recordor[_recorder_id] = msg;
    } // 赋值后, `t`不再存在于trace回溯链中
    Trace& operator=(Trace&& t) {
        msg = t.msg;
        _fl = t._fl;
        _l = t._l;
        _is_empty = _is_empty;
        _recorder_id = t._recorder_id;
        trace_recordor[_recorder_id] = this;
        trace_msg_recordor[_recorder_id] = msg;
        t.clear();      
    } // 赋值后, `t`不再存在于trace回溯链中
    ~Trace() {
        if(_is_empty == false && (*(trace_recordor.begin()+_recorder_id)) == this) {
            trace_recordor.erase(trace_recordor.begin()+_recorder_id);
            trace_msg_recordor.erase(trace_msg_recordor.begin()+_recorder_id);
        }
    }

    std::string message() const { return msg; };
    void clear() {
        _fl.clear();
        msg.clear();
        _l = -1;
        _recorder_id = -1;
        _is_empty = true;
    }

public:
    std::string _fl;
    int _l;
private:
    bool _is_empty;
    int _recorder_id;
    std::string msg;
};
#define CREATE_TRACE() Trace(__FILE__, __FUNCTION__, __LINE__)
#define CREATE_TRACE_BY_OFFSET(offset) Trace(__FILE__, __FUNCTION__, __LINE__, (offset))
#ifndef USE_MDEBUG
#define LOAD_TRACE()
#else
#if USE_MDEBUG == true
#define LOAD_TRACE() auto ______tmp_load = CREATE_TRACE_BY_OFFSET(1);
#else
#define LOAD_TRACE()
#endif
#endif
#ifndef USE_MDEBUG
#define REMOVE_TRACE()
#else
#if USE_MDEBUG == true
#define REMOVE_TRACE() ______tmp_load.clear();
#else
#define REMOVE_TRACE()
#endif
#endif
#ifndef USE_MDEBUG
#define RELOAD_TRACE()
#else
#if USE_MDEBUG == true
#define RELOAD_TRACE() ______tmp_load = CREATE_TRACE_BY_OFFSET(1);
#else
#define RELOAD_TRACE()
#endif
#endif
#ifndef USE_MDEBUG
#define MDEBUG(expr) expr
#else
#if USE_MDEBUG == true
#define MDEBUG(expr) auto ______tmp_debug = CREATE_TRACE_BY_OFFSET(0); expr; ______tmp_debug.clear()
#else
#define MDEBUG(expr) expr
#endif
#endif

class TraceWroker
{
public:
    static void error(int _up_trace_offset=0, int _down_trace_offset=0, const std::vector<std::string>& msg_recordor=trace_msg_recordor) {
        if(msg_recordor.empty()) exit(1);
        std::cout << RED << "--Error Trace Backward--" << RESET << std::endl;
        for(int i = 0; i < msg_recordor.size()-1; i++) {
            for(int j = 0; j < i; j++) {
                std::cout << "\t";
            }
            std::cout << msg_recordor[i];
            if(i == 0) {
                auto _info_str = Extra_File::read_lines(
                    trace_recordor[i]->_fl.c_str(),
                    trace_recordor[i]->_l,
                    0,
                    0
                );
                for(int j = 0; j <= i; j++) {
                    std::cout << "\t";
                }
                std::cout << RED << _info_str[0] << RESET << std::endl;
            }
        }

        auto _info_str = Extra_File::read_lines(
            trace_recordor[msg_recordor.size()-1]->_fl.c_str(),
            trace_recordor[msg_recordor.size()-1]->_l,
            _up_trace_offset,
            _down_trace_offset
        );
        for(int j = 0; j < msg_recordor.size()-1; j++) {
            std::cout << "\t";
        }
        std::cout << msg_recordor[msg_recordor.size()-1];
        for(int i = 0; i < (_up_trace_offset+_down_trace_offset+1); i++) {
            for(int j = 0; j < msg_recordor.size(); j++) {
                std::cout << "\t";
            }
            if((i-_up_trace_offset) == 0) {
                std::cout << RED << _info_str[i] << RESET << std::endl;
            } else {
                std::cout << _info_str[i] << std::endl;
            }
        }
        exit(1); // 退出
    }

    static void warning(const std::vector<std::string>& msg_recordor=trace_msg_recordor) {
        if(msg_recordor.empty()) return;
        std::cout << YELLOW << "--Warning Trace--" << RESET << std::endl;
        std::cout << YELLOW << *(msg_recordor.begin()+(msg_recordor.size()-1)) << RESET;
    }
};
#define ERROR() TraceWroker::error(3, 3)
#define ERROR_BY_OFFSET(up_offset, down_offset) TraceWroker::error((up_offset), (down_offset))
#define WARN() TraceWroker::warning()

#ifndef USE_MDEBUG
#define IndexCheck(check_index, max_index) \
    if(check_index < -max_index || check_index >= max_index) { \
        ERROR(); \
    }
#else
#if USE_MDEBUG == true
#define IndexCheck(check_index, max_index) \
    Trace ______tmp_index; \
    if(check_index >= -max_index && check_index < max_index) { \
        ______tmp_index = CREATE_TRACE(); \
    } else { \
        ______tmp_index = CREATE_TRACE(); \
        ERROR(); \
    }
#else
#define IndexCheck(check_index, max_index) \
    if(check_index < -max_index || check_index >= max_index) { \
        ERROR(); \
    }
#endif
#endif


#ifndef USE_MDEBUG
#define SizeCheck(size) \
    if(size <= 0) { \
        ERROR(); \
    }
#else
#if USE_MDEBUG == true
#define SizeCheck(size) \
    Trace ______tmp_size; \
    if(size > 0) { \
        ______tmp_size = CREATE_TRACE(); \
    } else { \
        ______tmp_size = CREATE_TRACE(); \
        ERROR(); \
    }
#else
#define SizeCheck(size) \
    if(size <= 0) { \
        ERROR(); \
    }
#endif
#endif


#ifndef USE_MDEBUG
#define EqualCheck(left, right) \
    if(left != right) { \
        ERROR(); \
    }
#else
#if USE_MDEBUG == true
#define EqualCheck(left, right) \
    Trace ______tmp_equal; \
    if(left == right) { \
        ______tmp_equal = CREATE_TRACE(); \
    } else { \
        ______tmp_equal = CREATE_TRACE(); \
        ERROR(); \
    }
#else
#define EqualCheck(left, right) \
    if(left != right) { \
        ERROR(); \
    }
#endif
#endif


#ifndef USE_MDEBUG
#define ValueCheck(value, down, up) \
    if(value < down && value > up) { \
        ERROR(); \
    }
#else
#if USE_MDEBUG == true
#define ValueCheck(value, down, up) \
    Trace ______tmp_value; \
    if(value >= down && value <= up) { \
        ______tmp_value = CREATE_TRACE(); \
    } else { \
        ______tmp_value = CREATE_TRACE(); \
        ERROR(); \
    }
#else
#define ValueCheck(value, down, up) \
    if(value < down && value > up) { \
        ERROR(); \
    }
#endif
#endif


#ifndef USE_MDEBUG
#define ZeroCheck(value) \
    if(value == 0) { \
        ERROR(); \
    }
#else
#if USE_MDEBUG == true
#define ZeroCheck(value) \
    Trace ______tmp_zero; \
    if(value != 0) { \
        ______tmp_zero = CREATE_TRACE(); \
    } else { \
        ______tmp_zero = CREATE_TRACE(); \
        ERROR(); \
    }
#else
#define ZeroCheck(value) \
    if(value == 0) { \
        ERROR(); \
    }
#endif
#endif

#ifndef USE_MDEBUG
#define Zero2Check(value1, value2) \
    if(value1 == 0 || value2 == 0) { \
        ERROR(); \
    }
#else
#if USE_MDEBUG == true
#define Zero2Check(value1, value2) \
    Trace ______tmp_zero2; \
    if(value1 != 0 && value2 != 0) { \
        ______tmp_zero2 = CREATE_TRACE(); \
    } else { \
        ______tmp_zero2 = CREATE_TRACE(); \
        ERROR(); \
    }
#else
#define Zero2Check(value1, value2) \
    if(value1 == 0 || value2 == 0) { \
        ERROR(); \
    }
#endif
#endif


#endif
