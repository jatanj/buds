#include "cli.h"

namespace buds {

CLI::Option* addOption(CLI::App* cmd, ArgsMap* args, const std::string& name)
{
    return cmd->add_option_function<std::string>(name, [args, name](auto value) {
        args->emplace(name, value);
    });
}

CliArgs parseArgs(std::vector<std::string> argv) {
  CLI::App cli{PROGRAM_DESCRIPTION, PROGRAM_NAME};
  ArgsMap args;

  addOption(&cli, &args, OPTION_CONFIG_FILE);
  addOption(&cli, &args, OPTION_ADDRESS);

  cli.parse(std::move(argv));

  ArgsConverter ex{args};

  CliArgs result{};
  result.configFile = ex.string(OPTION_CONFIG_FILE);
  result.address = ex.string(OPTION_ADDRESS);
  return result;
}

} // namespace buds
