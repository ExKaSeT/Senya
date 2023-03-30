#include "logger_builder_concrete.h"
#include "logger_concrete.h"

logger_builder *logger_builder_concrete::add_stream(
    std::string const &path,
    logger::severity severity)
{
    _construction_info[path] = severity;

    return this;
}

logger *logger_builder_concrete::construct() const
{
    return new logger_concrete(_construction_info);
}
