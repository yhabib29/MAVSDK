// WARNING: THIS FILE IS AUTOGENERATED! As such, it should not be edited.
// Edits need to be made to the proto files
// (see https://github.com/mavlink/MAVSDK-Proto/blob/master/protos/log_files/log_files.proto)

#include <iomanip>

#include "log_files_impl.h"
#include "plugins/log_files/log_files.h"

namespace mavsdk {

using ProgressData = LogFiles::ProgressData;
using Entry = LogFiles::Entry;

LogFiles::LogFiles(System& system) : PluginBase(), _impl{new LogFilesImpl(system)} {}

LogFiles::LogFiles(std::shared_ptr<System> system) : PluginBase(), _impl{new LogFilesImpl(system)}
{}

LogFiles::~LogFiles() {}

void LogFiles::get_entries_async(const GetEntriesCallback callback)
{
    _impl->get_entries_async(callback);
}

std::pair<LogFiles::Result, std::vector<LogFiles::Entry>> LogFiles::get_entries() const
{
    return _impl->get_entries();
}

void LogFiles::download_log_file_async(
    uint32_t id, std::string path, DownloadLogFileCallback callback)
{
    _impl->download_log_file_async(id, path, callback);
}

bool operator==(const LogFiles::ProgressData& lhs, const LogFiles::ProgressData& rhs)
{
    return ((std::isnan(rhs.progress) && std::isnan(lhs.progress)) || rhs.progress == lhs.progress);
}

std::ostream& operator<<(std::ostream& str, LogFiles::ProgressData const& progress_data)
{
    str << std::setprecision(15);
    str << "progress_data:" << '\n' << "{\n";
    str << "    progress: " << progress_data.progress << '\n';
    str << '}';
    return str;
}

bool operator==(const LogFiles::Entry& lhs, const LogFiles::Entry& rhs)
{
    return (rhs.id == lhs.id) && (rhs.date == lhs.date) && (rhs.size_bytes == lhs.size_bytes);
}

std::ostream& operator<<(std::ostream& str, LogFiles::Entry const& entry)
{
    str << std::setprecision(15);
    str << "entry:" << '\n' << "{\n";
    str << "    id: " << entry.id << '\n';
    str << "    date: " << entry.date << '\n';
    str << "    size_bytes: " << entry.size_bytes << '\n';
    str << '}';
    return str;
}

std::ostream& operator<<(std::ostream& str, LogFiles::Result const& result)
{
    switch (result) {
        case LogFiles::Result::Unknown:
            return str << "Unknown";
        case LogFiles::Result::Success:
            return str << "Success";
        case LogFiles::Result::Next:
            return str << "Next";
        case LogFiles::Result::NoLogfiles:
            return str << "No Logfiles";
        case LogFiles::Result::Timeout:
            return str << "Timeout";
        case LogFiles::Result::InvalidArgument:
            return str << "Invalid Argument";
        case LogFiles::Result::FileOpenFailed:
            return str << "File Open Failed";
        default:
            return str << "Unknown";
    }
}

} // namespace mavsdk