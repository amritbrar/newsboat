#include <matcher.h>
#include <logger.h>
#include <cassert>

#include <sys/time.h>
#include <ctime>

namespace newsbeuter {

matchable::matchable() { }
matchable::~matchable() { }

matcher::matcher() { }

bool matcher::parse(const std::string& expr) {
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);

	bool b = p.parse_string(expr);

	gettimeofday(&tv2, NULL);
	unsigned long diff = (((tv2.tv_sec - tv1.tv_sec) * 1000000) + tv2.tv_usec) - tv1.tv_usec;
	GetLogger().log(LOG_DEBUG, "matcher::parse: parsing `%s' took %lu µs (success = %d)", expr.c_str(), diff, b ? 1 : 0);

	return b;
}

bool matcher::matches(matchable* item) {
	bool retval = false;
	if (item) {
		struct timeval tv1, tv2;
		gettimeofday(&tv1, NULL);

		retval = matches_r(p.get_root(), item);

		gettimeofday(&tv2, NULL);
		unsigned long diff = (((tv2.tv_sec - tv1.tv_sec) * 1000000) + tv2.tv_usec) - tv1.tv_usec;
		GetLogger().log(LOG_DEBUG, "matcher::matches matching took %lu µs", diff);
	}
	return retval;
}

bool matcher::matches_r(expression * e, matchable * item) {
	if (e) {
		bool retval;
		switch (e->op) {
			case LOGOP_AND:
				retval = matches_r(e->l, item);
				retval = retval && matches_r(e->r, item); // short-circuit evaulation in C -> short circuit evaluation in the filter language
				break;
			case LOGOP_OR:
				retval = matches_r(e->l, item);
				retval = retval || matches_r(e->r, item); // same here
				break;
			case MATCHOP_EQ:
				if (item->has_attribute(e->name))
					retval = (item->get_attribute(e->name)==e->literal);
				else
					retval = false;
				break;
			case MATCHOP_NE:
				if (item->has_attribute(e->name))
					retval = (item->get_attribute(e->name)!=e->literal);
				else
					retval = false;
				break;
			case MATCHOP_RXEQ:
				retval = false; // TODO: implement
				break;
			case MATCHOP_RXNE:
				retval = false;
				break;
			default:
				GetLogger().log(LOG_ERROR, "matcher::matches_r: invalid operator %d", e->op);
				assert(false); // that's an error condition
		}
		return retval;
	} else {
		return true; // shouldn't happen
	}
}

}
