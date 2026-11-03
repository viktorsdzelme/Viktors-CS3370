//Viktors Dzelme
// Reflection: Q1: If std::optional<KeyValue> is returned, then read_raw wouldn't be able to tell the difference between a blank line and a malformed line. If std::optional<KeyValue, ConfigError> is returned, then invalid inputs could look valid.

// Q2: A constructor has no return value. what() is for people to read, while error(). code is for code to branch on safely.”


#include "pulse_config.h"

#include <cassert>
#include <fstream>
#include <utility>

namespace {

template <typename T, typename Convert>
std::expected<T, ParseError> parse_number(std::string_view s, Convert convert) {
    try {
        const std::string str{s};          
        std::size_t used = 0;
        const T value = convert(str, &used);
        if (used != str.size())            
            return std::unexpected(ParseError::NotANumber);
        return value;
    } catch (const std::invalid_argument&) {
        return std::unexpected(ParseError::NotANumber);
    } catch (const std::out_of_range&) {
        return std::unexpected(ParseError::OutOfRange);
    }
}

}  

std::expected<int, ParseError> parse_int(std::string_view s) {
    return parse_number<int>(s, [](const std::string& str, std::size_t* pos) {
        return std::stoi(str, pos);
    });
}

std::expected<double, ParseError> parse_double(std::string_view s) {
    return parse_number<double>(s, [](const std::string& str, std::size_t* pos) {
        return std::stod(str, pos);
    });
}

namespace {

std::string_view trim(std::string_view s) {
    constexpr std::string_view blanks = " \t\r";
    const auto first = s.find_first_not_of(blanks);
    if (first == std::string_view::npos)
        return {};                                   
    const auto last = s.find_last_not_of(blanks);
    return s.substr(first, last - first + 1);
}

}  

std::expected<std::optional<KeyValue>, ConfigError>
parse_line(std::string_view text, int line_no) {
    text = trim(text);

    if (text.empty() || text.front() == '#')
        return std::optional<KeyValue>{};

    const auto eq = text.find('=');
    const std::string_view key =
        (eq == std::string_view::npos) ? std::string_view{} : trim(text.substr(0, eq));
    if (key.empty())                                 
        return std::unexpected(
            ConfigError{ConfigErrc::MalformedLine, std::string{text}, line_no});

    const std::string_view value = trim(text.substr(eq + 1));
    return std::optional<KeyValue>{
        KeyValue{std::string{key}, std::string{value}, line_no}};
}

std::expected<void, ConfigError> RawParams::set(const KeyValue& kv) {
    std::optional<std::string>* slot = nullptr;
    if      (kv.key == "vt")               slot = &vt;
    else if (kv.key == "width")            slot = &width;
    else if (kv.key == "pulse_delta")      slot = &pulse_delta;
    else if (kv.key == "drop_ratio")       slot = &drop_ratio;
    else if (kv.key == "below_drop_ratio") slot = &below_drop_ratio;

    if (!slot)
        return std::unexpected(
            ConfigError{ConfigErrc::UnknownKey, kv.key, kv.line});
    if (*slot)                                       
        return std::unexpected(
            ConfigError{ConfigErrc::DuplicateKey, kv.key, kv.line});

    *slot = kv.value;
    return {};
}

std::expected<RawParams, ConfigError> read_raw(std::istream& in) {
    RawParams raw;
    std::string line;
    int line_no = 0;
    while (std::getline(in, line)) {
        ++line_no;
        auto parsed = parse_line(line, line_no);
        if (!parsed)                                 
            return std::unexpected(std::move(parsed.error()));
        if (!*parsed)                                
            continue;
        auto stored = raw.set(**parsed);             
        if (!stored)
            return std::unexpected(std::move(stored.error()));
    }
    return raw;
}

namespace {

ConfigErrc to_errc(ParseError e) {
    switch (e) {
        case ParseError::NotANumber: return ConfigErrc::NotANumber;
        case ParseError::OutOfRange: return ConfigErrc::OutOfRange;
    }
    return ConfigErrc::NotANumber;   
}

template <typename T, typename Parse, typename InRange>
std::expected<T, ConfigError>
get_field(const std::optional<std::string>& text, const char* key,
          Parse parse, InRange in_range) {
    assert(text.has_value());        
    auto value = parse(*text);
    if (!value)
        return std::unexpected(ConfigError{to_errc(value.error()), key, 0});
    if (!in_range(*value))           
        return std::unexpected(ConfigError{ConfigErrc::OutOfRange, key, 0});
    return *value;
}

}  

std::expected<PulseParams, ConfigError> validate(const RawParams& raw) {
    if (!(raw.vt && raw.width && raw.pulse_delta &&
          raw.drop_ratio && raw.below_drop_ratio))
        return std::unexpected(ConfigError{ConfigErrc::MissingKeys, "", 0});

    
    auto positive     = [](int x)    { return x >= 1; };
    auto non_negative = [](int x)    { return x >= 0; };
    auto fraction     = [](double x) { return x > 0.0 && x < 1.0; };

    auto vt = get_field<int>(raw.vt, "vt", parse_int, positive);
    if (!vt) return std::unexpected(std::move(vt.error()));

    auto width = get_field<int>(raw.width, "width", parse_int, positive);
    if (!width) return std::unexpected(std::move(width.error()));

    auto delta = get_field<int>(raw.pulse_delta, "pulse_delta", parse_int, positive);
    if (!delta) return std::unexpected(std::move(delta.error()));

    auto ratio = get_field<double>(raw.drop_ratio, "drop_ratio", parse_double, fraction);
    if (!ratio) return std::unexpected(std::move(ratio.error()));

    auto below = get_field<int>(raw.below_drop_ratio, "below_drop_ratio",
                                parse_int, non_negative);
    if (!below) return std::unexpected(std::move(below.error()));

    return PulseParams{*vt, *width, *delta, *ratio, *below};
}

std::expected<PulseParams, ConfigError> load_params(std::istream& in) {
    auto raw = read_raw(in);
    if (!raw)
        return std::unexpected(std::move(raw.error()));
    return validate(*raw);
}

std::string to_message(const ConfigError& err, std::string_view filename) {
    const std::string file{filename};   
    switch (err.code) {                 
        case ConfigErrc::CannotOpen:
            return "Cannot open INI file: " + file;
        case ConfigErrc::MalformedLine:
            return "Malformed line " + std::to_string(err.line) + " in " + file;
        case ConfigErrc::UnknownKey:
        case ConfigErrc::DuplicateKey:
            return "Invalid or duplicate INI key: " + err.detail;
        case ConfigErrc::MissingKeys:
            return "Invalid INI file: missing one or more keys";
        case ConfigErrc::NotANumber:
        case ConfigErrc::OutOfRange:
            return "Invalid value for " + err.detail + " in " + file;
    }
    return {};                          
}

ConfigFileError::ConfigFileError(ConfigError err, std::string_view filename)
    : std::runtime_error(to_message(err, filename)),
      err_(std::move(err)) {}

PulseConfig::PulseConfig(const std::string& filename) {
    std::ifstream in{filename};
    if (!in)
        throw ConfigFileError{
            ConfigError{ConfigErrc::CannotOpen, filename, 0}, filename};

    auto loaded = load_params(in);
    if (!loaded)
        throw ConfigFileError{std::move(loaded.error()), filename};

    params_ = *loaded;                  
}
