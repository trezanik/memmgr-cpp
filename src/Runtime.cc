
/**
 * @file	Runtime.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */


#include "Allocator.h"		// we own, create, and destroy
#include "Runtime.h"		// prototypes


// Global/Singleton assignment for application access
Runtime	&runtime = Runtime::Instance();



Runtime::Runtime()
{
}



Runtime::~Runtime()
{
}



#if defined(USING_MEMORY_DEBUGGING)

Allocator*
Runtime::Memory() const
{
	static Allocator	mem;
	return &mem;
}

#endif
