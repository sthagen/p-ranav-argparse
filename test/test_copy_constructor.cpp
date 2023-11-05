#ifdef WITH_MODULE
import argparse;
#else
#include <argparse/argparse.hpp>
#endif
#include <doctest.hpp>
#include <test_utility.hpp>

using doctest::test_suite;

TEST_CASE("Parse positional arguments using a copy of an ArgumentParser" *
          test_suite("vector")) {

  auto maker = []() {
    argparse::ArgumentParser program("test");
    program.add_argument("first");
    program.add_argument("second").nargs(2);

    return program;
  };

  auto program = maker();

  REQUIRE_NOTHROW(program.parse_args(
      {"test", "rocket.mesh", "thrust_profile.csv", "config.json"}));

  auto first = program.get<std::string>("first");
  REQUIRE(first == "rocket.mesh");
  auto second = program.get<std::vector<std::string>>("second");
  REQUIRE(second.size() == 2);
  REQUIRE(second[0] == "thrust_profile.csv");
  REQUIRE(second[1] == "config.json");
}

TEST_CASE("Parse optional arguments using a copy of an ArgumentParser" *
          test_suite("vector")) {

  auto maker = []() {
    argparse::ArgumentParser program("test");
    program.add_argument("--first");
    program.add_argument("--second").nargs(2);

    return program;
  };

  auto program = maker();

  REQUIRE_NOTHROW(
      program.parse_args({"test", "--first", "rocket.mesh", "--second",
                          "thrust_profile.csv", "config.json"}));

  auto first = program.get<std::string>("--first");
  REQUIRE(first == "rocket.mesh");
  auto second = program.get<std::vector<std::string>>("--second");
  REQUIRE(second.size() == 2);
  REQUIRE(second[0] == "thrust_profile.csv");
  REQUIRE(second[1] == "config.json");
}

TEST_CASE("Segmentation fault on help (Issue #260)") {

  struct SubparserContainer {
    argparse::ArgumentParser parser;
  };

  auto get_container = []() {
    SubparserContainer *container = nullptr;
    if (container == nullptr) {
      argparse::ArgumentParser parser("subcommand", "1.0",
                                      argparse::default_arguments::all, false);
      parser.add_description("Example");
      container = new SubparserContainer{parser};
    }
    return container;
  };

  argparse::ArgumentParser program("program");
  auto *container = get_container();
  program.add_subparser(container->parser);

  std::ostringstream oss;
  std::streambuf *p_cout_streambuf = std::cout.rdbuf();
  std::cout.rdbuf(oss.rdbuf());

  program.parse_args({"program", "subcommand", "-h"});

  std::cout.rdbuf(p_cout_streambuf); // restore

  auto cmdline_output = oss.str();
  REQUIRE(cmdline_output.size() > 0);
  REQUIRE(cmdline_output.find("shows help message and exits") !=
          std::string::npos);
}

TEST_CASE("Segmentation fault on custom help (Issue #260)") {

  struct SubparserContainer {
    argparse::ArgumentParser parser;
  };

  auto get_container = []() {
    SubparserContainer *container = nullptr;
    if (container == nullptr) {
      argparse::ArgumentParser parser("subcommand", "1.0",
                                      argparse::default_arguments::none, false);
      parser.add_description("Example");
      std::string temporary{"temp+string"};
      parser.add_argument("-h", "--help")
          .flag()
          .nargs(0)
          .action(
              [&](const auto &) -> void { std::cout << temporary << "\n"; });

      container = new SubparserContainer{parser};
    }
    return container;
  };

  argparse::ArgumentParser program("program");
  auto *container = get_container();
  program.add_subparser(container->parser);

  std::ostringstream oss;
  std::streambuf *p_cout_streambuf = std::cout.rdbuf();
  std::cout.rdbuf(oss.rdbuf());

  program.parse_args({"program", "subcommand", "-h"});

  std::cout.rdbuf(p_cout_streambuf); // restore

  auto cmdline_output = oss.str();
  REQUIRE(cmdline_output.size() > 0);
  REQUIRE(cmdline_output.find("temp+string") != std::string::npos);
}