#include "logger/logger.h"
#include "logger/logger_builder_concrete.h"

int main()
{
    logger_builder* builder = new logger_builder_concrete();

    logger *constructed_logger = builder
        ->add_stream("file1.txt", logger::severity::critical)
        ->add_stream("file2.txt", logger::severity::debug)
        ->add_stream("file3.txt", logger::severity::trace)
        ->construct();

    logger *constructed_logger_2 = builder
        ->add_stream("file4.txt", logger::severity::warning)
        ->construct();

    constructed_logger
        ->log("kek lol 123", logger::severity::information);

    constructed_logger_2
        ->log("123 kek lol", logger::severity::error);

    delete constructed_logger_2;
    delete constructed_logger;
    delete builder;

    return 0;
}