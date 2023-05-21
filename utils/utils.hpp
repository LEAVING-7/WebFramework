#pragma once
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace wf::utils {
template <typename... Args>
auto print(const std::string_view fmt, Args&&... args)
{
  auto fmtArgs = std::make_format_args(args...);
  fputs(std::vformat(fmt, fmtArgs).c_str(), stdout);
}
template <typename... Args>
auto println(const std::string_view fmt, Args&&... args)
{
  auto fmtArgs = std::make_format_args(args...);
  fputs(std::vformat(fmt, fmtArgs).c_str(), stdout);
  fputs("\n", stdout);
}
inline auto StringSplit(std::string_view str, char delim) -> std::vector<std::string>
{
  auto elems = std::vector<std::string>();
  auto lastPos = str.find_first_not_of(delim, 0);
  auto pos = str.find_first_of(delim, lastPos);
  while (pos != std::string::npos || lastPos != std::string::npos) {
    elems.emplace_back(str.substr(lastPos, pos - lastPos));
    lastPos = str.find_first_not_of(delim, pos);
    pos = str.find_first_of(delim, lastPos);
  }
  return elems;
}
inline auto StringJoin(std::span<std::string> strs, std::string_view delim) -> std::string
{
  auto result = std::string {};
  for (size_t i = 0; i < strs.size(); ++i) {
    result += strs[i];
    if (i != strs.size() - 1) {
      result += delim;
    }
  }
  return result;
}
} // namespace wf::utils