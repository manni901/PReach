#ifndef STL_H_INCLUDED
#define STL_H_INCLUDED

#define FOREACH_STL(el, list)																		\
	for(typeof(list.begin()) it = list.begin(); it != list.end(); it++){	\
	typeof(*it)& el = *it;
#ifndef END_FOREACH
#define END_FOREACH }
#endif

#endif // STL_H_INCLUDED