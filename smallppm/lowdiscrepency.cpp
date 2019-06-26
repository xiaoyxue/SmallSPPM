#include "lowdiscrepency.h"

int IsPrime(int a) noexcept {
	ASSERT(a >= 2);
	for (int i = 2; i * i <= a; i++) {
		if (a % i == 0)
			return false;
	}
	return true;
}