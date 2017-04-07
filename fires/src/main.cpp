#include <Windows.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "../inc/everything.h"


bool IsEndWith(const std::string& src, const std::string& test)
{
	size_t src_len = src.length();
	size_t test_len = test.length();
	if (src_len < test_len)
	{
		return false;
	}
	std::string end_str = src.substr(src_len - test_len, src_len - 1);
	return end_str == test;
}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 3)
	{
		std::cout << "usage: ff command_name" << std::endl;
		std::cout << "       ff command_name $1" << std::endl;
		exit(-1);
	}

	char* prog = nullptr;
	char* file_name = nullptr;

	if (argc == 2) 
	{
		prog = argv[1];
	}
	else if (argc == 3) 
	{
		prog = argv[1];
		file_name = argv[2];
	}
	Everything_SetSearch(prog);

	BOOL query_statue = Everything_Query(TRUE);
	if (query_statue == FALSE)
	{
		DWORD error_code = Everything_GetLastError();
		switch (error_code)
		{
		case EVERYTHING_ERROR_MEMORY:
			std::cout << "Failed to allocate memory for the search query." << std::endl;
			break;
		case EVERYTHING_ERROR_IPC:
			std::cout << "server not available" << std::endl;
			break;
		case EVERYTHING_ERROR_CREATETHREAD:
			std::cout << "Failed to create the search query thread." << std::endl;
			break;
		case EVERYTHING_ERROR_INVALIDCALL:
			std::cout << "Invalid call." << std::endl;
			break;
		default:
			std::cout << "faild with unknow reason." << std::endl;
			break;
		}
		exit(EXIT_FAILURE);
	}

	std::vector<std::string> result{};
	{
		DWORD match_file_result = Everything_GetNumFileResults();
		char buf[1024] = { 0 };
		std::string path;
		for (DWORD i = 0; i < match_file_result; i++)
		{
			Everything_GetResultFullPathName(i, buf, sizeof(buf) / sizeof(char));
			path.assign(buf);
			if (IsEndWith(path, ".exe"))
			{
				result.push_back(path);
			}
			path.clear();
			memset(buf, 0, sizeof(buf));
		}
		std::sort(result.begin(), result.end(), [](auto a, auto b) {
			return a.length() < b.length();
		});
	}

	auto size = result.size();
	if (size == 0)
	{
		std::cout << "No result found." << std::endl;
		std::cout.flush();
		exit(EXIT_FAILURE);
	}

	size_t current_size = size > 15 ? 15 : size;
	for (auto i = 0; i < current_size; ++i)
	{
		std::cout << "[";
		std::cout.width(3);
		std::cout << i << " ] " << result[i] << std::endl;
	}
	size_t new_current_size = 15;
	int index = -1;
	if (size > 1)
	{
		std::string your_choice;
		std::cout << "Select to start: ";
		std::cout.flush();
		while (std::getline(std::cin, your_choice))
		{
			try
			{
				if (your_choice == "q" || your_choice == "Q")
				{
					std::cout << "Exited." << std::endl;
					std::cout.flush();
					exit(EXIT_SUCCESS);
				}

				if (your_choice == "n" || your_choice == "N")
				{
					if (current_size > size - 1)
					{
						std::cout << "End" << std::endl;
						std::cout.flush();
						exit(EXIT_FAILURE);
					}
					current_size = new_current_size;
					new_current_size = current_size + 15;
					new_current_size = new_current_size > size ? size : new_current_size;
					for (auto i = current_size; i < new_current_size; ++i)
					{
						std::cout << "[";
						std::cout.width(3);
						std::cout << i << " ] " << result[i] << std::endl;
					}
					goto END;
				}
				index = std::stoi(your_choice);
			}
			catch (std::exception& ex)
			{
				std::cout << ex.what() << std::endl;
				goto END;
			}

			if (!(index >= 0 && index <= result.size()))
			{
				std::cout << "wrong choice!" << std::endl;
			}

			break;

		END:
			std::cout << "Select to start: ";
			std::cout.flush();
		}
	}
	else
	{
		index = 0;
	}



	if (file_name) 
	{
		result[index].append(" ").append(file_name);
	}
	char cmd[1024] = {0};

	result[index].copy(cmd, 1204);

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	try
	{
		if (!CreateProcessA(nullptr,
			cmd,
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			nullptr,
			&si,
			&pi))
		{
			std::cout << "Error Exit!" << std::endl;
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}


	return 0;
}
