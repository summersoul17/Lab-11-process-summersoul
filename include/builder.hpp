// Copyright 2020 Your Name <your_email>

#ifndef BUILDER_BUILDER_HPP_
#define BUILDER_BUILDER_HPP_

#include <common.hpp>

class timer;

class builder {
 public:
  builder(const builder&) = delete;
  builder();

  int vmain(boost::program_options::variables_map&& vm);

  boost::program_options::variables_map parse_console_args(int argc,
                                                           char** argv);
  std::string print_help();
  void timeout_handler();
  void read_settings(boost::program_options::variables_map&& vm);
  bool spawn_proc(const std::string& target);
 private:
  boost::program_options::options_description _desc{"Allowed options"};
  std::unique_ptr<settings> _psettings;
  std::unique_ptr<thread_data> _pdata;
  std::unique_ptr<timer> _ptimer;
};

class timer {
 private:
  std::thread _timer_thread;
 public:
  timer() = default;
  timer(timer&& timer_) noexcept {
    _timer_thread = std::move(timer_._timer_thread);
  }
  timer(std::chrono::seconds delay, builder* callback_obj):
      _timer_thread([=](){
        std::this_thread::sleep_for(std::chrono::seconds(delay));
        (*callback_obj).timeout_handler();
      }){}
  ~timer(){ if(_timer_thread.joinable()) _timer_thread.detach(); }
};


#endif // INCLUDE_BUILDER_HPP_
