#pragma once
#include <string>

namespace UI {
    // Opens a native Windows folder selection dialog.
    // Returns the selected path or empty string if cancelled.
    std::string select_folder(const std::string& title);
}
