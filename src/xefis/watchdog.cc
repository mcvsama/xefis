/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standards:
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// System:
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <locale.h>
#include <string.h>
#include <errno.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/fail.h>


enum class PingLoopResult
{
	Exited,
	Timeout,
};


PingLoopResult
ping_loop (int write_fd, int read_fd, pid_t xefis_pid)
{
	// Read should be non-blocking:
	::fcntl (read_fd, F_SETFL, O_NONBLOCK);
	::srand (::time (nullptr));

	// First delay is slightly longer:
	usleep ((1_s).quantity<Microsecond>());

	while (true)
	{
		// Send ping, receive pong.

		uint8_t c = ::rand() % 0x100;
		write (write_fd, &c, 1);
		fsync (write_fd);

		// Give xefis some time:
		usleep ((100_ms).quantity<Microsecond>());

		// if Xefis exits normally, wait(), cleanup and return Exited
		// otherwise return Timeout
		uint8_t r = 0;
		size_t n = read (read_fd, &r, 1);
		if (n == 0 || (r ^ 0x55) != c)
		{
			int status = 0;

			switch (waitpid (xefis_pid, &status, WNOHANG))
			{
				case 0:
					return PingLoopResult::Timeout;

				case -1:
					// Error. Just retry.
					break;

				default:
					if (WIFEXITED (status))
						return PingLoopResult::Exited;
					else
						return PingLoopResult::Timeout;
			}
		}
	}

	return PingLoopResult::Exited;
}


int
watchdog (int argc, char** argv, char**)
{
	std::vector<const char*> args (argv, argv + argc);

	int w_fd_for_watchdog = -1;
	int r_fd_for_watchdog = -1;
	int w_fd_for_xefis = -1;
	int r_fd_for_xefis = -1;

	while (true)
	{
		// A new set of pipes should be generated every time.
		{
			int pipes[2];

			// Watchdog -> Xefis pipe:
			if (::pipe (pipes) != 0)
			{
				std::cerr << "Watchdog: Couldn't create pipe for communication with Xefis: " << strerror (errno) << std::endl;
				return EXIT_FAILURE;
			}

			w_fd_for_watchdog = pipes[1];
			r_fd_for_xefis = pipes[0];

			// Xefis -> Watchdog pipe:
			if (::pipe (pipes) != 0)
			{
				std::cerr << "Watchdog: Couldn't create pipe for communication with Xefis: " << strerror (errno) << std::endl;
				return EXIT_FAILURE;
			}

			w_fd_for_xefis = pipes[1];
			r_fd_for_watchdog = pipes[0];
		}

		pid_t xefis_pid = fork();
		switch (xefis_pid)
		{
			case -1:
				std::cerr << "Watchdog: Failed to create subprocess: " << strerror (errno) << std::endl;
				break;

			case 0:
			{
				std::vector<const char*> cmdline = args; // ./watchdog[0] xefis[1] --other[2] ...[...]...
				// No need to care about allocated memory, since we'll be replaced soon by another program.
				cmdline.erase (cmdline.begin());
				cmdline.push_back (::strdup (("--watchdog-write-fd=" + std::to_string (w_fd_for_xefis)).c_str()));
				cmdline.push_back (::strdup (("--watchdog-read-fd=" + std::to_string (r_fd_for_xefis)).c_str()));
				cmdline.push_back (nullptr); // Sentinel for execv.

				std::cerr << "Watchdog: Executing: ";
				for (const char* s: cmdline)
					if (s)
						std::cerr << s << " ";
				std::cerr << std::endl;

				execv (cmdline[0], const_cast<char* const*> (cmdline.data()));
				// Failure?
				std::cerr << "Watchdog: Failed to load program: " << strerror (errno) << std::endl;
				abort();
				break;
			}

			default:
				switch (ping_loop (w_fd_for_watchdog, r_fd_for_watchdog, xefis_pid))
				{
					case PingLoopResult::Exited:
						return EXIT_SUCCESS;

					case PingLoopResult::Timeout:
						std::cerr << "Watchdog: timeout; restarting" << std::endl;
						kill (xefis_pid, SIGKILL);
						break;
				}
				break;
		}

		// Wait a bit and try again:
		usleep ((10_ms).quantity<Microsecond>());

		::close (w_fd_for_watchdog);
		::close (r_fd_for_watchdog);
		::close (w_fd_for_xefis);
		::close (r_fd_for_xefis);
	}

	return EXIT_SUCCESS;
}


int
main (int argc, char** argv, char** envp)
{
	signal (SIGILL, xf::fail);
	signal (SIGFPE, xf::fail);
	signal (SIGSEGV, xf::fail);

	setenv ("LC_ALL", "POSIX", 1);
	setlocale (LC_ALL, "POSIX");

	int result = watchdog (argc, argv, envp);
	std::cerr << "Watchdog exits." << std::endl;
	return result;
}

