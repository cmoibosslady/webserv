#include <algorithm>
#include <cctype>
#include <cerrno>
#include <exception>
#include <limits>
#include <stdexcept>
# include "stoul.hpp"

static int ft_isdigit(const int c)
{
	return (c >= '0' && c <= '9');
}

static int ft_isupperhexa(const int c)
{
	return (c >= 'A' && c <= 'F');
}

static int ft_islowerhexa(const int c)
{
	return (c >= 'a' && c <= 'f');
}

static int	ft_isspace(const int c)
	//Very basic implementation of isspace function, as we are not allowed to use the original. Here it uses logical OR (||) (cf. logic operator in c++).
{
	return (c == ' ' || (c >= 9 && c <= 13));
}

int		is_a_numeric_string(const std::string & s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && ft_isdigit(*it)) it++;
	return (!s.empty() && it == s.end());
}

unsigned long	ft_strtoul(const char *__restrict__ str, char **__restrict__ endptr, int base)
	// Restric is a keyword that tell the compiler that the pointer is the only reference to the data it points to. No other pointer will be used to access the object. It helps the compiler to make optimizations (see vectorization).
	// Basic C function strtoul declared the two pointers with restrict. However in C++ restrict keyword is not 'translated'. Hence the use of __restrict__ to tell the compiler to treat it as restrict. Might not work depending on the compiler.
{
	while (ft_isspace(*str))
		str++;
	if (*str == '+') // We don't handle negative numbers for unsigned long conversion.
		str++;
	if (base == 16)
	{
		if (*str == '0' && str[1])
			if (str[1] == 'x' || str[1] == 'X')
				str += 2; //skip the "0x" or "0X" prefix for hexadecimal numbers.
	}
	errno = 0;
	unsigned long	result = 0;
	while (1)
	{
		if (ft_isdigit(*str))
		{
			if ((*str - '0') >= (int)base)
				break ;
			result = result * base + (*str - '0');
		}
		else if (base == 16 && ft_isupperhexa(*str))
		{
			result = result * base + (*str - 'A' + 10);
		}
		else if (base == 16 && ft_islowerhexa(*str))
		{
			result = result * base + (*str - 'a' + 10);
		}
		else
			break ;
		//overflow check
		if (result > (std::numeric_limits<unsigned long>::max() - 15) / base)
		{
			errno = ERANGE;
			result = std::numeric_limits<unsigned long>::max();
			break ;
		}
		str++;
	}
	endptr ? *endptr = (char *)str : 0;
	return (result);
}

unsigned long	ft_stoul(std::string const & str, size_t *idx, int base)
	//std::stoul is a c++11 function converting str to unsigned long. Since we are using c++98, we have to implement our own version.
{
	char	*endp;
	unsigned long value = ft_strtoul(str.c_str(), &endp, base);

	if (endp == str.c_str())
		throw std::invalid_argument("ft_stoul: invalid argument");
	if (value == std::numeric_limits<unsigned long>::max() && errno == ERANGE)
		throw std::out_of_range("ft_stoul: out of range");
	if (idx)
		*idx = endp - str.c_str();
	return (value);
}

// Test for the ft_stoul function:
// 	size_t idx = 0;
// 	try {
// 		std::cout << ft_stoul(argv[1], &idx, 16) << "with idx=" << idx << std::endl;
// 		idx = 0;
// 		std::cout << std::stoul(argv[1], &idx, 16) << "with idx=" << idx << std::endl;
// 	}
// 	catch (std::exception &e) {
// 		std::cerr << "Exception: " << e.what() << std::endl;
// 	}


static char safe_tolower(char ch)
{
	return (static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
}

std::string &	my_tolower(std::string & str)
{
	std::transform(str.begin(), str.end(), str.begin(), safe_tolower);
	return (str);
}

