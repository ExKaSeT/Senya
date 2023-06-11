#include <fstream>
#include <sstream>
#include "logger_builder_concrete.h"
#include "logger_concrete.h"

logger_builder* logger_builder_concrete::add_stream(
	std::string const& path,
	logger::severity severity)
{
	_construction_info[path] = severity;

	return this;
}

logger* logger_builder_concrete::construct() const
{
	return new logger_concrete(_construction_info);
}

logger* logger_builder_concrete::file_construct(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	logger_builder* builder = new logger_builder_concrete();

	std::string line;
	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string path_str, severity_str;
		std::getline(ss, path_str, ';');
		std::getline(ss, severity_str);

		logger::severity severity = logger_concrete::severityFromString(severity_str);
		builder->add_stream(path_str, severity);
	}
	auto logger = builder->construct();
	delete builder;
	return logger;
}
