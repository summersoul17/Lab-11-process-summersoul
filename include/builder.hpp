// Copyright 2020 Your Name <your_email>

#ifndef INCLUDE_BUILDER_HPP_
#define INCLUDE_BUILDER_HPP_

#include <common.hpp>
#include <string>
#include <memory>
#include <utility>

class timer;

class builder {
 public:
  builder(const builder&) = delete;
  builder();

  int vmain(boost::program_options::variables_map&& vm);

  boost::program_options::variables_map parse_console_args(int argc,
                                                           char** argv);
  std::string print_help();
  void read_settings(boost::program_options::variables_map&& vm);
  bool spawn_proc(const std::string& target, thread_data& _pdata);
 private:
  boost::program_options::options_description _desc{"Allowed options"};
  std::unique_ptr<settings> _psettings;
};

class timer {
 private:
  std::thread _timer_thread;

 public:
  timer() = default;
  timer(timer&& t) { _timer_thread = std::move(t._timer_thread); }
  timer(std::chrono::seconds delay,
        std::function<void(thread_data&)> callback_obj,
        thread_data& pdata)
      : _timer_thread([delay, callback_obj, &pdata]() {
          std::this_thread::sleep_for(std::chrono::seconds(delay));
          callback_obj(pdata);
        }) {}
  ~timer() {
    if (_timer_thread.joinable()) _timer_thread.detach();
  }
};


#endif // INCLUDE_BUILDER_HPP_
