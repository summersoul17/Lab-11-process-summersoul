#include <builder.hpp>
#include <iostream>


int main(int argc, char** argv) {
  builder b{};
  log_setup::init();
  return b.vmain(b.parse_console_args(argc, argv));
}