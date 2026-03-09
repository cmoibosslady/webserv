#ifndef POLLER_HPP
# define POLLER_HPP

# include <map>
# include <poll.h>
# include <vector>
# include "main.hpp"

class Poller
{
	public:
		Poller(void);
		Poller(const Poller &other);
		Poller&	operator=(const Poller &other);
		~Poller(void);

		poll_status	add(int fd, short events);
		poll_status	remove(int fd);
		poll_status	modify(int fd, short events);
		int	wait(int timeout);

	private:
		std::vector<struct pollfd>	_fds;
		nfds_t						_nfds; // Size of _fds
		std::map<int, size_t>		_fdIndexMap;
};

#endif
