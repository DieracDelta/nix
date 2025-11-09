#include "nix/main/loggers.hh"
#include "nix/util/environment-variables.hh"
#include "nix/main/progress-bar.hh"

namespace nix {

LogFormat defaultLogFormat = LogFormat::raw;

LogFormat parseLogFormat(const std::string & logFormatStr)
{
    static const std::map<std::string_view, LogFormat> formatMap = {
        {"raw", LogFormat::raw},
        {"raw-with-logs", LogFormat::rawWithLogs},
        {"internal-json", LogFormat::internalJSON},
        {"bar", LogFormat::bar},
        {"bar-with-logs", LogFormat::barWithLogs}};

    if (getEnv("NIX_GET_COMPLETIONS"))
        return LogFormat::raw;

    for (const auto & [name, fmt] : formatMap)
        if (name == logFormatStr)
            return fmt;

    std::string validValues;
    for (const auto & [name, _] : formatMap) {
        if (!validValues.empty())
            validValues += ", ";
        validValues += name;
    }

    throw Error("option 'log-format' has an invalid value '%s'. Valid values are %s.", logFormatStr, validValues);
}

std::unique_ptr<Logger> makeDefaultLogger()
{
    switch (defaultLogFormat) {
    case LogFormat::raw:
        return makeSimpleLogger(false);
    case LogFormat::rawWithLogs:
        return makeSimpleLogger(true);
    case LogFormat::internalJSON:
        return makeJSONLogger(getStandardError());
    case LogFormat::bar:
        return makeProgressBar();
    case LogFormat::barWithLogs: {
        auto logger = makeProgressBar();
        logger->setPrintBuildLogs(true);
        return logger;
    }
    default:
        unreachable();
    }
}

void setLogFormat(const std::string & logFormatStr)
{
    setLogFormat(parseLogFormat(logFormatStr));
}

void setLogFormat(const LogFormat & logFormat)
{
    defaultLogFormat = logFormat;
    logger = makeDefaultLogger();
}

} // namespace nix
