#ifndef RUNTIME_H_INCLUDED
#define RUNTIME_H_INCLUDED

/**
 * @file	Runtime.h
 * @author	James Warren
 * @brief	Application globally-accessible singleton containing other classes
 */



// forward declarations
class Allocator;



/**
 * Dedicated class for storing all 'global' variables. This itself is accessed
 * through an extern by the runtime code, so is in itself actually a global.
 *
 * The classes accessed are not constructed until their first call, which keeps
 * the application startup brief. You can therefore make assumptions in the
 * child classes about the lifetimes of other objects, and safely know each
 * will have their destructor called before the runtime is destroyed.
 *
 * @class Runtime
 */
class Runtime
{
private:
	// no class assignment or copy
	Runtime operator = (Runtime const&);
	Runtime(Runtime const&);

	// private constructor and destructor; automatically available
	Runtime();
	~Runtime();

public:

	/**
	 * Acquires the singleton reference to the class. Only used within
	 * Runtime.cc in order to make the runtime accessible globally (accessed
	 * by 'runtime') - shouldn't be used outside of this.
	 */
	static Runtime& Instance()
	{
		static Runtime	rtime;
		return rtime;
	}


#if defined(USING_MEMORY_DEBUGGING)
	/**
	 * Retrieves a pointer to the static Allocator variable.
	 *
	 * @return Always returns a pointer to an Allocator instance; never
	 * fails
	 */
	Allocator*
	Memory() const;
#endif

};



// extern; Runtime.cc
extern Runtime		&runtime;



#endif	// RUNTIME_H_INCLUDED
