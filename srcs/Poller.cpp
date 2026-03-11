#include <algorithm>
#include "ft_mem.hpp"
#include "main.hpp"
#include "main.tpp"
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
	struct pollfd	new_fd = {.fd = fd, .events = events, .revents = 0};
	_fds.push_back(new_fd);
	_fdIndexMap[fd] = _fds.size() - 1;
	_nfds++;
	return POLL_SUCCESS;
}

poll_status Poller::remove(int fd) {
	size_t index = _fdIndexMap[fd];
	_fdIndexMap[_fds[_fds.size() - 1].fd] = index;
	_fds[index] = _fds[_fds.size() - 1];
	_fds.pop_back();
	_fdIndexMap.erase(fd);
	_nfds--;
	return POLL_SUCCESS;
}

poll_status	Poller::modify(int fd, short events) {
	size_t index = _fdIndexMap[fd];
	_fds[index].events = events;
	return POLL_SUCCESS;
}

std::vector<int>	Poller::wait(int timeout) {
	poll(_fds.data(), _nfds, timeout);
}
