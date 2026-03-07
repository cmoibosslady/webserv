#include <algorithm>
#include "ft_mem.hpp"
#include "main.hpp"
#include "Poller.hpp"

Poller::Poller(void): _nfds(0) {
	log_info("Poller instance created");
}

Poller::Poller(const Poller & other) {
	*this = other;
	log_info("Poller instance copied");
}

Poller&	Poller::operator=(const Poller & other) {
	this->_fds = other._fds;
	this->_nfds = other._nfds;
	return *this;
}

Poller::~Poller(void) {
	log_info("Poller instance destroyed");
}

poll_status Poller::add(int fd, short events) {
	if (_nfds + 1 == MAX_EVENTS)
		return POLL_FAILURE;
	struct pollfd	new_fd = {.fd = fd, .events = events};
	_fds.push_back(new_fd);
	_nfds++;
	return POLL_SUCCESS;
}

poll_status Poller::remove(int fd) {
	size_t index = findFd(fd);
	if (index == _fds.size())
		return POLL_FAILURE;
	_fds[findFd(fd)] = _fds[_fds.size() - 1];
	_fds.pop_back();
	_nfds--;
	return POLL_SUCCESS;
}

poll_status	Poller::modify(int fd, short events) {
	size_t index = findFd(fd);
	if (index == _fds.size())
		return POLL_FAILURE;
	_fds[index].events = events;
	return POLL_SUCCESS;
}

int	Poller::wait(int timeout) {
	return (poll(_fds.data(), _nfds, timeout));
}

size_t	Poller::findFd(const int fd) {
	for (size_t i = 0; i < _fds.size(); i++) {
		if (_fds[i].fd == fd)
			return i;
	}
	return _fds.size();
}
