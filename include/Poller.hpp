#ifndef POLLER_HPP
# define POLLER_HPP

# include <poll.h>

class Poller
{
	public:
		Poller(void);
		Poller(const Poller &other);
		Poller&	operator=(const Poller &other);
		~Poller(void);

		void	add(int fd, short events);
		void	remove(int fd);
		void	modify(int fd, short events);
		int		wait(struct pollfd *fds, nfds_t nfds, int timeout);

	private:
		struct pollfd	_fds[MAX_EVENTS];
		nfds_t			_nfds;
};

#endif
