#ifndef STOUL_HPP
# define STOUL_HPP

# include <string>

int	is_a_numeric_string(const std::string &s);
unsigned long	ft_strtoul(const char *__restrict__ str, char **__restrict__ endptr, int base);
unsigned long	ft_stoul(std::string const & str, size_t *idx = 0, int base = 10);

#endif
