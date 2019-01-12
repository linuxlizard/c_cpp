#ifndef NETRC_HPP
#define NETRC_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

// https://en.cppreference.com/w/cpp/feature_test
#ifdef __has_include
#  if __has_include(<filesystem>)
#    include <filesystem>  // gcc8 (Fedora29+)
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem> // gcc7 (Ubuntu 18.04)
#  endif
#else
#error no __has_include
#endif

// key: machine's value
// value: tuple of login,password,account
// (account is optional)
using netrc_map  = std::map<std::string, std::tuple<std::string, std::string, std::string>>;

class parse_error : public std::exception
{
public:
	parse_error (std::string& failure) noexcept
	{
		this->line_failed = failure;
	}
	
	virtual const char* what() const throw()
	{ 
		return this->line_failed.c_str();
	}

private:
	std::string line_failed;
};

class file_error : public std::exception
{
	public:
		file_error (const std::string& netrc_filename) noexcept
		{
			this->filename = netrc_filename;
		}

		virtual const char *what() const throw()
		{
			return this->filename.c_str();
		}

	private:
		std::string filename;
};

netrc_map netrc_parse(std::istream& infile);
netrc_map netrc_parse_file(const std::string& infilename);

#endif

