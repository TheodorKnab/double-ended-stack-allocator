/**
* Exercise: "DoubleEndedStackAllocator with Canaries" OR "Growing DoubleEndedStackAllocator with Canaries (VMEM)"
* Group members: Robert Barta (gsXXXX), Theodor Knab (gsXXXX), Oliver Schmidt (gsXXXX)
**/

#include "stdio.h"
#include <cstdlib>
#include <cstdint>
#include <ostream>
#include <iostream>
#include <assert.h>  

#define OFFSET_SIZE sizeof(uint32_t)
#define CANARY_SIZE sizeof(uint32_t)
namespace Tests
{
	void Test_Case_Success(const char* name, bool passed)
	{
		if (passed)
		{
			printf("[%s] passed the test!\n", name);
		}
		else
		{
			printf("[%s] failed the test!\n", name);
		}
	}

	void Test_Case_Failure(const char* name, bool passed)
	{	
		if (!passed)
		{
			printf("[%s] passed the test!\n", name);
		}
		else
		{
			printf("[%s] failed the test!\n", name);
		}
	}

	/** 
	* Example of how a test case can look like. The test cases in the end will check for
	* allocation success, proper alignment, overlaps and similar situations. This is an
	* example so you can already try to cover all cases you judge as being important by
	* yourselves.
	**/
	template<class A>
	bool VerifyAllocationSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);

		if (mem == nullptr)
		{
			printf("[Error]: Allocator returned nullptr!\n");
			return false;
		}

		return true;
	}

	// Free Success

	template<class A>
	bool VerifyFreeSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);

		allocator.Free(mem);

		void* mem2 = allocator.Allocate(size, alignment);

		if (mem != mem2)
		{
			printf("[Error]: Allocate -> Free -> Allocate did not return the same address for first and second allocation!\n");
			return false;
		}

		return true;
	}
	
	// Allocate Back Success
	template<class A>
	bool VerifyAllocationBackSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.AllocateBack(size, alignment);

		if (mem == nullptr)
		{
			printf("[Error]: Allocator returned nullptr!\n");
			return false;
		}

		return true;
	}
	
	// Free Back Success

	template<class A>
	bool VerifyFreeBackSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.AllocateBack(size, alignment);

		allocator.FreeBack(mem);

		void* mem2 = allocator.AllocateBack(size, alignment);

		if (mem != mem2)
		{
			printf("[Error]: Allocate Back -> Free Back -> Allocate Back did not return the same address for first and second allocation!\n");
			return false;
		}

		return true;
	}
	
	// Allocate Align Success
	   
	// Allocate Back Align Success
	   
	// Allocate -> Allocate (different Alignment) -> Free -> Free
	   
	// Allocate Back -> Allocate Back (different Alignment) -> Free Back -> Free Back
	   
	// Allocate -> Reset -> Allocate
	   
	// Allocate Back -> Reset -> Allocate Back
	   
	// Free out of bounds
	   
	// Free nullptr
	   
	// Free Back out of bounds
	   
	// Free Back nullptr
	   
	// Free Empty Allocator
	   
	// Free Back Empty Allocator
	   
	// Allocate MaxSize
	   
	// Allocate Back MaxSize
	   
	// Allocate Front & Back Half Size
	   
	// Allocate Front into Back Memory
	   
	// Allocate Back into Front Memory
	   
	// Allocate uint_32
	   
	// Allocate back uint_32
	   
	// Overwrite Canaries Front (Modify DEADBEEF)
	   
	// Overwrite Canaries Back (Modify DEADBEEF)
	   
	// Alignment + Size > max_size
	   
	// Allocate with Size 0
	   
	// Allocate with Alignment 0

	// Initialize Allocator with max_size <= 12
		
}

// Assignment functionality tests are going to be included here 

#define WITH_DEBUG_CANARIES 1

/**
* You work on your DoubleEndedStackAllocator. Stick to the provided interface, this is
* necessary for testing your assignment in the end. Don't remove or rename the public
* interface of the allocator. Also don't add any additional initialization code, the
* allocator needs to work after it was created and its constructor was called. You can
* add additional public functions but those should only be used for your own testing.
**/
class DoubleEndedStackAllocator
{
public:
	DoubleEndedStackAllocator(size_t max_size)
	{
		m_begin = reinterpret_cast<char*>(malloc(max_size));
		m_end = reinterpret_cast<char*>(m_begin) + max_size;
		m_current = m_begin;
		m_current_back = m_end;
	}

	// Overhead of 4 bytes normally
	// Overhead of 12 bytes with debug canaries
	void* Allocate(size_t size, size_t alignment)
	{
		uint32_t total_allocation_size = size + OFFSET_SIZE;
		uint32_t front_memory_offset = OFFSET_SIZE;
		
#if WITH_DEBUG_CANARIES
		total_allocation_size += CANARY_SIZE * 2;
		front_memory_offset += CANARY_SIZE;
#endif
		
		// If there is not enough memory, we return a nullptr
		if (!FitsInAllocator(total_allocation_size, alignment)) return nullptr;

		uint32_t offset = reinterpret_cast<char*>(m_current) - reinterpret_cast<char*>(m_begin);
		
		// Align address up with offset added, then revert offset so the actual user data is aligned
		m_current = AlignUp(reinterpret_cast<char*>(m_current) + front_memory_offset, alignment) - front_memory_offset;		
		void* allocated_address = reinterpret_cast<char*>(m_current) + front_memory_offset;
		
		// Store offset in first bytes
		uint32_t* m_current_uint = reinterpret_cast<uint32_t*>(m_current);
		*m_current_uint = offset;

		//set m_current to new address
		m_current = reinterpret_cast<char*>(m_current) + front_memory_offset  + size;

#if WITH_DEBUG_CANARIES
		//place DEADBEEF at the front of the allocated memory
		uint32_t* front_debug_canaray = m_current_uint + 1;
		*front_debug_canaray = 0xDEADBEEF;
		
		//place DEADBEEF at the back of the allocated memory and adjust m_current
		uint32_t* back_debug_canaray = reinterpret_cast<uint32_t*>(m_current);
		*back_debug_canaray = 0xDEADBEEF;

		//adjust m_current
		m_current = back_debug_canaray + 1;
#endif

		return allocated_address;
	}

	void* AllocateBack(size_t size, size_t alignment)
	{
		uint32_t total_allocation_size = size + OFFSET_SIZE;
		uint32_t front_memory_offset = OFFSET_SIZE;

#if WITH_DEBUG_CANARIES
		total_allocation_size += CANARY_SIZE * 2;
		front_memory_offset += CANARY_SIZE;
#endif

		// If there is not enough memory, we return a nullptr
		if (!FitsInAllocator(total_allocation_size, alignment)) return nullptr;

		uint32_t offset = reinterpret_cast<char*>(m_end) - reinterpret_cast<char*>(m_current_back);

		// Create space for the allocated object
		m_current_back = reinterpret_cast<char*>(m_current_back) - size;

#if WITH_DEBUG_CANARIES
		 m_current_back = reinterpret_cast<char*>(m_current_back) - CANARY_SIZE;
#endif
		
		// Align address down
		m_current_back = AlignDown(reinterpret_cast<char*>(m_current_back), alignment);
		void* allocated_address = reinterpret_cast<char*>(m_current_back);

		// Store offset in first bytes

#if WITH_DEBUG_CANARIES
		uint32_t* m_current_back_uint = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current_back) - OFFSET_SIZE - CANARY_SIZE);
#else
		uint32_t* m_current_back_uint = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current_back) - OFFSET_SIZE);
#endif

		*m_current_back_uint = offset;


#if WITH_DEBUG_CANARIES
		//place DEADBEEF at the front of the allocated memory
		uint32_t* front_debug_canaray = m_current_back_uint + 1;
		*front_debug_canaray = 0xDEADBEEF;

		//place DEADBEEF at the back of the allocated memory and adjust m_current
		uint32_t* back_debug_canaray = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current_back) + size);
		*back_debug_canaray = 0xDEADBEEF;

		//adjust m_current
		m_current_back_uint = front_debug_canaray - 1;
#endif

		//set m_current_back to new address
		m_current_back = m_current_back_uint;

		return allocated_address;
	}

	void Free(void* memory)
	{
		// Frees the given memory block by looking up the previous address and shifting m_current to its position.
		
		// ASSERT if a canary was changed
#if WITH_DEBUG_CANARIES
		uint32_t* front_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - CANARY_SIZE);
		uint32_t* back_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current) - CANARY_SIZE);
		assert("canaries violated -> memory leak", *front_canary == 0xDEADBEEF && *back_canary == 0xDEADBEEF);
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE - CANARY_SIZE);
#else 		
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE);
#endif

		m_current = reinterpret_cast<char*>(m_begin) + *offset_ptr;
	}

	void FreeBack(void* memory)
	{

#if WITH_DEBUG_CANARIES
		uint32_t* back_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - CANARY_SIZE);
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE - CANARY_SIZE);
#else 		
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE);
#endif

		m_current_back = reinterpret_cast<char*>(m_end) - *offset_ptr;

#if WITH_DEBUG_CANARIES
		uint32_t* front_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current_back) - CANARY_SIZE);
		assert("canaries violated -> memory leak", *front_canary == 0xDEADBEEF && *back_canary == 0xDEADBEEF);
#endif

	}

	void Reset(void)
	{
		
	}

	~DoubleEndedStackAllocator(void) {}

private:
	void* m_begin;
	void* m_end;
	void* m_current;
	void* m_current_back;
		
	bool FitsInAllocator(size_t size, size_t alignment) { return true; }

	// Aligns the pointer address up in respect to the specified alignment
	char* AlignUp(char* address, size_t alignment)
	{
		const size_t alignment_mask = ~(alignment - 1);
		address = reinterpret_cast<char*>(reinterpret_cast<uint64_t>(address + (alignment - 1)) & alignment_mask);

		return address;
	}

	// Aligns the pointer address down in respect to the specified alignment
	char* AlignDown(char* address, size_t alignment)
	{
		const size_t alignment_mask = ~(alignment - 1);
		address = reinterpret_cast<char*>(reinterpret_cast<uint64_t>(address) & alignment_mask);
		return address;
	}
};


int main()
{
	// You can add your own tests here, I will call my tests at the end with a fresh instance of your allocator and a specific max_size
	{
		// You can remove this, just showcasing how the test functions can be used
		DoubleEndedStackAllocator allocator(1024u);
		//Tests::Test_Case_Success("Allocate() returns nullptr", allocator.Allocate(32, 1) == nullptr);

		// Test Case 1 - Assertion that allocator aligns correctly
		//DoubleEndedStackAllocator testcase1(1024u);
		//void* tc1_address1 = allocator.Allocate(6, 4);
		//void* tc1_address2 = allocator.Allocate(6, 4);
		//uint32_t result = reinterpret_cast<char*>(tc1_address2) - reinterpret_cast<char*>(tc1_address1);
		//bool tc1_condition = (result == 4);
		//Tests::Test_Case_Success("Allocate aligns correctly", tc1_condition);
		// -------------------------------------------------------
		//Tests::Test_Case_Success("Allocate() does not return nullptr", [&allocator]() { return Tests::VerifyAllocationBackSuccess(allocator, 32, 5); }());
		Tests::Test_Case_Success("Allocate() does not return nullptr", Tests::VerifyAllocationSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("AllocateBack() does not return nullptr", Tests::VerifyAllocationBackSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("Free() resets to the correct address", Tests::VerifyFreeSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("FreeBack() resets to the correct address", Tests::VerifyFreeBackSuccess(allocator, 32, 4));
	}

	// You can do whatever you want here in the main function

	// Here the assignment tests will happen - it will test basic allocator functionality. 
	{

	}
}