#pragma once
#include <string>

namespace Gimp {
bool launchWithImage(const std::wstring& imagePath);
std::wstring findExecutable();
bool exist();
}  // namespace Gimp
