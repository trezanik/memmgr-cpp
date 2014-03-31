#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

/**
 * @file	Allocator.h
 * @author	James Warren
 * @brief	Memory Allocation tracking for the application
 */


/* bring in this definition from a 'build.h' or ./configure setting. For now, we
 * specify it here. Adjust for your own project as necessary. We also define the
 * other definitions also needed */
#define USING_MEMORY_DEBUGGING
#define DISABLE_MEMORY_CHECK_TO_STDOUT	// no spam - toggle on/off
#define DISABLE_MEMORY_OP_TO_STDOUT


#if defined(USING_MEMORY_DEBUGGING)

#include "Runtime.h"			// technical dependency for macros
#include <mutex>			// C++11 mutex, locking
#include <vector>			// storage container
#define __STDC_FORMAT_MACROS
#include <cinttypes>			// for PRIxPTR


// Optional: Enclose in namespace here


// forward declarations
class Object;

// required definitions
#define MEM_LEAK_LOG_NAME		"memdynamic.log"
#define MEM_MAX_FILENAME_LENGTH		31
#define MEM_MAX_FUNCTION_LENGTH		31

/* don't ask. I did this a while ago and had issues (I believe it was with
 * visual studio, as usual), this is what I came up with for a workaround, and
 * it works, so... */
#if defined(_WIN64)
#	define PRINT_POINTER	"%016p"
#elif defined(_WIN32)
#	define PRINT_POINTER	"%08p"
#elif defined(__x86_64__)
#	define PRINT_POINTER	"%016" PRIxPTR " "
#else
#	define PRINT_POINTER	"%08" PRIxPTR
#endif



/**
 * This structure is added to the end of an allocated block of memory, when
 * USING_MEMORY_DEBUGGING is defined.
 *
 * This ensures no data has been written beyond the boundary of an allocated bit
 * of memory, resulting in heap/stack corruption.
 *
 * @struct memblock_footer
 */
struct memblock_footer
{
	unsigned	magic;		/**< footer magic number */
};


/**
 * This structure is added at the start of a block of allocated memory, when
 * USING_MEMORY_DEBUGGING is defined.
 *
 * @struct memblock_header
 */
struct memblock_header
{
	/**
	 * The header magic number is used to detect if an operation on memory has
	 * written into this structure. Corrupt headers usually mean something else
	 * has written into the block - an exception being if an operation has
	 * stepped too far back, to the extent of overwriting the magic number */
	unsigned		magic;

	/** A pointer to this memory blocks footer */
	memblock_footer*	footer;

	/** A pointer to the object that allocated the memory. If there was none,
	 * i.e. it was a standalone function, this member is a nullptr. */
	const Object*		owner;

	/**
	 * The file this memory block was created in; is not allocated dynamically,
	 * and so is bound by the @a MEM_MAX_FILENAME_LENGTH definition.
	 */
	char		file[MEM_MAX_FILENAME_LENGTH+1];
	/** The function name this memory block was created in */
	char		function[MEM_MAX_FUNCTION_LENGTH+1];
	/** The line in the file this memory block was created in */
	uint32_t	line;
	/** The size, in bytes, the original request desired */
	uint32_t	requested_size;
	/** The size, in bytes, of the total allocation (header+data+footer) */
	uint32_t	real_size;
};



/**
 * Memory-specific error codes. Used only as the return value from CheckBlock().
 *
 * @enum E_MEMORY_ERROR
 */
enum E_MEMORY_ERROR
{
	EC_NoError = 0,
	EC_NoMemoryBlock,
	EC_CorruptHeader,
	EC_CorruptFooter,
	EC_SizeMismatch
};



/**
 * Resides within the application runtime and tracks all memory allocations and
 * frees, ensuring data is not corrupt or the memory leaked.
 *
 * This overrides all new/delete calls so they will NOT be usable,
 * ensuring the correct functions are always called for the application. As
 * system headers should always be included before this one, it will not stop
 * normal API functions from working - if you do have a problem, check your
 * inclusion order!
 *
 * malloc/free and associates are not overriden, enabling their usage in certain
 * special cases where using this class would be inappropriate or infeasible -
 * such as allocating for this class in the first place.
 *
 * @class Allocator
 */
class Allocator
{
	/* we are created on the stack in Runtime::Memory(); needs access to
	 * call our private constructor */
	friend class Runtime;
private:
	// no class assignment or copy
	Allocator operator = (Allocator const&);
	Allocator(Allocator const&);

	// private constructor; we want one instance that is controlled.
	Allocator();


	uint32_t	_allocs;		/**< The amount of times new has been called successfully */
	uint32_t	_frees;			/**< The amount of times delete has been called successfully */
	uint32_t	_current_allocated;	/**< Currently allocated amount of bytes */
	uint32_t	_total_allocated;	/**< The total amount of allocated bytes */

	/**
	 * The lock for making modifications to _memblocks. Mutable to allow
	 * constness for retrieval functions, recursive for multi-locking.
	 */
	mutable std::recursive_mutex	_mutex;

	/**
	 * Performance benchmarks have shown that a vector is fastest when the
	 * size of the element is known, for insert, remove, and search!
	 * http://www.baptiste-wicht.com/2012/12/cpp-benchmark-vector-list-deque/
	 *
	 * The small data type (a pointer) is most beneficial here.
	 */
	std::vector<memblock_header*>	_memblocks;


	/**
	 * Called only in the destructor; will always output the memory stats
	 * for the application run, but will also write out the information on
	 * any unfreed memory.
	 *
	 * Outputs to MEM_LEAK_LOG_NAME, but if it's not writable, it is printed
	 * to stderr instead.
	 */
	void
	OutputMemoryInfo();


public:
	~Allocator();


	/**
	 * Checks a block of memory, ensuring both the header and footer are not
	 * corrupt, and the rest of the block matches the original requestors
	 * specifications.
	 *
	 * Define DISABLE_MEMORY_CHECK_TO_STDOUT to prevent stage-by-stage
	 * output of the checking process.
	 *
	 * @param[in] memory_block The block of memory to check
	 * @retval E_MEMORY_ERROR The relevant memory error code as to the block
	 * status; should hopefully always be EC_NoError
	 */
	E_MEMORY_ERROR
	CheckBlock(
		memblock_header* memory_block
	) const;


	/**
	 */
	void*
	TrackedAlloc(
		const uint32_t num_bytes,
		const char* file,
		const char* function,
		const uint32_t line,
		const Object* owner = nullptr
	);


	/**
	 */
	void
	TrackedFree(
		void* memory
	);


	/**
	 *
	 * @param[in] memory_block The pointer to memory returned by TrackedAlloc()
	 * @param[in] num_bytes The new number of bytes to allocate
	 * @param[in] file The file this function was called in
	 * @param[in] function The function this function was called in
	 * @param[in] line The line number in the file this function was called in
	 * @param[in] owner The owner of the memory block
	 * @retval nullptr if the function fails due to invalid parameters, or
	 * the call to realloc fails
	 * @return A pointer to the usable block of memory allocated
	 */
	void*
	TrackedRealloc(
		void* memory_block,
		const uint32_t num_bytes,
		const char* file,
		const char* function,
		const uint32_t line,
		const Object* owner = nullptr
	);


	/**
	 *
	 * @param memory [in] the block of memory (as supplied by this function)
	 * to validate
	 * @retval true if the memory is not corrupt and usable
	 * @retval false if the memory failed one of the checks
	 */
	bool
	ValidateMemory(
		void* memory
	) const;
};



/* I like C-styles memory allocation. It's something you can rely on to always
 * behave in a certain way, no surprises, and not wrought with stupid limits on
 * what you can and can't do - ala C++.
 *
 * Only thing I don't like is the need to cast everything returned.
 *
 * While I can just redefine 'malloc' and 'free', depending on inclusion order
 * this may pick up more than we desire, and potentially break some other code
 * that we don't maintain. Style guidelines also decree a macro is ALL CAPS!
 */

/** Macro to create tracked memory */
	#define MALLOC(size)		runtime.Memory()->TrackedAlloc(size, __FILE__, __FUNCTION__, __LINE__)

/** Macro to delete tracked memory */
	#define FREE(varname)		runtime.Memory()->TrackedFree(varname)

/** The amount of bytes (limit) printed to output under the Data field */
	#define MEM_OUTPUT_LIMIT	1024


#else
	/* if we're here, we don't want to debug the memory, so just set the
	 * macros to call the original, non-hooked functions. */

// Optional: Enclose namspace here

	// note: Windows pollution defines DELETE, so you can never use that!
#	define MALLOC(size)		malloc(size)
#	define FREE(varname)		free(varname)

#endif	// USING_MEMORY_DEBUGGING


// Optional: Close namespace here


#endif	// ALLOCATOR_H_INCLUDED
