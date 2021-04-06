// Copyright 2020 Your Name <your_email>

#include <builder.hpp>

builder::builder() {
  _desc.add_options()("help,h", "Shows help info")(
      "source,s", boost::program_options::value<std::string>(),
      "Sets directory where the project is")(
      "config,c", boost::program_options::value<std::string>(),
      "Sets build configuration <Debug/Release>. Debug is default")(
      "install,i", "Sets install step on")("pack,p", "Sets pack step on")(
      "timeout,t", boost::program_options::value<int>(),
      "Sets timer (seconds)");
}

void timeout_handler(thread_data&);
int builder::vmain(boost::program_options::variables_map&& vm) {
  if (vm.count("help")) {
    BOOST_LOG_TRIVIAL(info) << "Help called";
    std::cout << print_help();
    return 0;
  }

  read_settings(std::move(vm));

  thread_data _pdata{false, boost::process::child()};
  //  std::this_thread::sleep_for(std::chrono::seconds(10));
  std::unique_ptr<timer> _timer;
  if (_psettings->time() != -1) {
    _timer = std::make_unique<timer>(
        timer(std::chrono::seconds(_psettings->time()), timeout_handler, _pdata));
  }
  // execute processes
  try {
    std::this_thread::sleep_for(std::chrono::seconds(4));
    std::shared_future<bool> pack = std::async([this, &_pdata]() -> bool {
      return this->spawn_proc("config", _pdata);
    });

    pack.wait();
    if (pack.get()) {
      pack = std::async([this, &_pdata]() -> bool {
        return this->spawn_proc("build", _pdata);
      });
      pack.wait();
    }
    if (_psettings->install()) {
      if (pack.get()) {
        pack = std::async([this, &_pdata]() -> bool {
          return this->spawn_proc("install", _pdata);
        });
        pack.wait();
      }
    }
    if (_psettings->pack()) {
      if (pack.get()) {
        pack = std::async([this, &_pdata]() -> bool {
          return this->spawn_proc("pack", _pdata);
        });
        pack.wait();
      }
    }
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(fatal) << "ERROR: " << e.what();
  }

  return 0;
}

bool builder::spawn_proc(const std::string& target, thread_data& _pdata) {
//  std::this_thread::sleep_for(std::chrono::seconds(1));
  BOOST_LOG_TRIVIAL(debug) << " _pdata->_terminated: " << bool(_pdata._terminated);
  if (_pdata._terminated) {
    BOOST_LOG_TRIVIAL(debug) << "Previous process terminated. Returning..";
    return false;
  }
  BOOST_LOG_TRIVIAL(info) << "Spawning target building: " << target;

  boost::process::ipstream stream;
  auto cmake_path = boost::process::search_path("cmake");
  BOOST_LOG_TRIVIAL(info) << "Found cmake: " << cmake_path.string();
  boost::process::child child(
      std::string(cmake_path.string() + " " + _psettings->get_command(target)),
      boost::process::std_out > stream);

  BOOST_LOG_TRIVIAL(info) << "Setting new _pdata...";
  _pdata.set_bool(false);
  _pdata.set_child(std::move(child));

  for (std::string line;
       _pdata._current_child.running() && std::getline(stream, line);) {
    BOOST_LOG_TRIVIAL(info) << line;
  }

  _pdata._current_child.wait();

  auto exit_code = _pdata._current_child.exit_code();

  if (exit_code != 0) {
    BOOST_LOG_TRIVIAL(error) << "Non zero exit code. Exiting...";
    _pdata.set_bool(true);
    return false;
  } else {
    return true;
  }
}

boost::program_options::variables_map builder::parse_console_args(int argc,
                                                                  char** argv) {
  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv)
      .options(_desc)
      .allow_unregistered()
      .run();
  po::store(parsed, vm);
  po::notify(vm);
  return vm;
}

std::string builder::print_help() {
  std::stringstream out;
  out << "Simple cmake builder" << _desc << "\n\nCopyright 2021 Lamp\n";
  return out.str();
}


void timeout_handler(thread_data& _pdata) {
  BOOST_LOG_TRIVIAL(debug) << "Timer timeout. Stopping all child processes...";
  // Stop child processes
  try {
    if (_pdata._current_child.running()) {
      _pdata._current_child.terminate();
    }
      _pdata.set_bool(true);
      BOOST_LOG_TRIVIAL(debug) << "_pdata set: " << _pdata._terminated;

  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(fatal) << "Terminating error: " << e.what()
                             << " Process: " << _pdata._current_child.id();
  }
}

void builder::read_settings(boost::program_options::variables_map&& vm) {
  bool install = false, pack = false;
  std::string config = "Debug";
  int time = -1;
  if (vm.count("config")) {
    config = vm["config"].as<std::string>();
  }
  if (vm.count("install")) {
    install = true;
  }
  if (vm.count("pack")) {
    pack = true;
  }
  if (vm.count("timeout")) {
    BOOST_LOG_TRIVIAL(debug)
        << "Timeout args got: " << vm["timeout"].as<int>() << ". Setting timer";
    time = vm["timeout"].as<int>();
  }
  _psettings = std::make_unique<settings>(settings((config), install, pack, time));
}

void log_setup::init() {
  boost::log::add_console_log(
      std::cout, boost::log::keywords::format =
                     "[%TimeStamp%][%Severity%][%ThreadID%]: %Message%");
  boost::log::add_file_log(
      boost::log::keywords::file_name = "log_%N.log",
      boost::log::keywords::rotation_size = 10 * 1024 * 1024,
      boost::log::keywords::time_based_rotation =
          boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
      boost::log::keywords::format =
          "[%TimeStamp%][%Severity%][%ThreadID%]: %Message%");
}
