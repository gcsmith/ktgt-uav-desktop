// -----------------------------------------------------------------------------
// File:    CommandLine.h
// Authors: Garrett Smith
//
// Helper routines for command line handling.
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_COMMANDLINE__H_
#define _HELIVIEW_COMMANDLINE__H_

#include <boost/program_options.hpp>

#define S_ARG(name, desc) (name, po::value<string>(), desc)
#define N_ARG(name, desc) (name, desc)

// -----------------------------------------------------------------------------
template <typename T>
void optional_arg(
    boost::program_options::variables_map &vm,
    const std::string &name, T &var)
{
    if (vm.count(name))
        var = vm[name].as<T>();
}

#endif // _HELIVIEW_COMMANDLINE__H_

