#pragma once

#include <expected>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

enum class ParseError { NotANumber, OutOfRange };

std::expected<int, ParseError>    parse_int(std::string_view s);
std::expected<double, ParseError> parse_double(std::string_view s);

enum class ConfigErrc {
    CannotOpen,
    MalformedLine,
    UnknownKey,
    DuplicateKey,
    MissingKeys,
    NotANumber,
    OutOfRange
};

struct ConfigError {
    ConfigErrc  code;
    std::string detail;
    int         line = 0;
};

std::string to_message(const ConfigError& err,
                       std::string_view filename);

struct KeyValue {
    std::string key;
    std::string value;
    int         line = 0;
};

std::expected<std::optional<KeyValue>, ConfigError>
parse_line(std::string_view text, int line_no);

struct RawParams {
    std::optional<std::string> vt;
    std::optional<std::string> width;
    std::optional<std::string> pulse_delta;
    std::optional<std::string> drop_ratio;
    std::optional<std::string> below_drop_ratio;

    std::expected<void, ConfigError> set(const KeyValue& kv);
};

struct PulseParams {
    int    vt               = 0;
    int    width            = 0;
    int    pulse_delta      = 0;
    double drop_ratio       = 0.0;
    int    below_drop_ratio = 0;
};

std::expected<RawParams, ConfigError> read_raw(std::istream& in);
std::expected<PulseParams, ConfigError>
validate(const RawParams& raw);
std::expected<PulseParams, ConfigError>
load_params(std::istream& in);

class ConfigFileError : public std::runtime_error {
public:
    ConfigFileError(ConfigError err, std::string_view filename);
    const ConfigError& error() const noexcept { return err_; }
private:
    ConfigError err_;
};

class PulseConfig {
public:
    explicit PulseConfig(const std::string& filename);
    const PulseParams& params() const noexcept { return params_; }
private:
    PulseParams params_;
};
