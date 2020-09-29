/**
* Exercise: "DoubleEndedStackAllocator with Canaries" OR "Growing DoubleEndedStackAllocator with Canaries (VMEM)"
* Group members: Robert Barta (gsXXXX), Theodor Knab (gsXXXX), Oliver Schmidt (gsXXXX)
**/

#include "stdio.h"
#include <cstdlib>
#include <cstdint>

#define OFFSET_SIZE sizeof(uint32_t)

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
}

// Assignment functionality tests are going to be included here 

#define WITH_DEBUG_CANARIES 0

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

	void* Allocate(size_t size, size_t alignment)
	{ 
		// If there is not enough memory, we return a nullptr
		if (!FitsInAllocator(size + OFFSET_SIZE, alignment)) return nullptr;

		// Align address up with offset added, then revert offset so the actual user data is aligned
		m_current = AlignUp(reinterpret_cast<char*>(m_current) + OFFSET_SIZE, alignment) - OFFSET_SIZE;
		
		void* allocated_address = reinterpret_cast<char*>(m_current) + OFFSET_SIZE;

		
		// Store offset in first bytes
		uint32_t* m_currentUInt = reinterpret_cast<uint32_t*>(m_current);		
		uint32_t offset = reinterpret_cast<char*>(m_current) - reinterpret_cast<char*>(m_begin);
		*m_currentUInt = offset;

		//set m_current to new address
		m_current = reinterpret_cast<char*>(m_current) + size + OFFSET_SIZE;
		
		return allocated_address;
	}

	void* AllocateBack(size_t size, size_t alignment) { return nullptr; }

	void Free(void* memory)
	{
		// Frees the given memory block by looking up the previous address and shifting m_current to its position.
		uint32_t offset_ptr = *(reinterpret_cast<uintptr_t*>(memory) - OFFSET_SIZE);
		//m_current = m_begin + offset_ptr;
	}

	void FreeBack(void* memory) {}

	void Reset(void) {}

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
		DoubleEndedStackAllocator testcase1(1024u);
		void* tc1_address1 = allocator.Allocate(2, 4);
		void* tc1_address2 = allocator.Allocate(2, 4);
		uint32_t result = reinterpret_cast<char*>(tc1_address2) - reinterpret_cast<char*>(tc1_address1);
		bool tc1_condition = (result == 4);
		Tests::Test_Case_Success("Allocate aligns correctly", tc1_condition);
		// -------------------------------------------------------
	}

	// You can do whatever you want here in the main function

	// Here the assignment tests will happen - it will test basic allocator functionality. 
	{

	}
}