/**
* Exercise: "DoubleEndedStackAllocator with Canaries"
* Group members: Robert Barta (gs19m002), Theodor Knab (gs19m006), Oliver Schmidt (gs19m013)
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
		
		allocator.Free(mem);
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
		
		allocator.Free(mem2);
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

		allocator.FreeBack(mem);
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
		
		allocator.FreeBack(mem2);
		return true;
	}
	
	// Allocate Align Success
	template<class A>
	bool VerifyAlignSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);

		if (reinterpret_cast<uint64_t>(mem) % alignment != 0)
		{
			printf("[Error]: Allocate -> did not return a correctly aligned address!\n");
			return false;
		}

		allocator.Free(mem);
		return true;
	}
	
	// Allocate Back Align Success
	template<class A>
	bool VerifyAlignBackSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.AllocateBack(size, alignment);

		if (reinterpret_cast<uint64_t>(mem) % alignment != 0)
		{
			printf("[Error]: AllocateBack -> did not return a correctly aligned address!\n");
			return false;
		}
		
		allocator.FreeBack(mem);
		return true;
	}

	// Allocate -> Allocate (different Alignment) -> Free -> Free	
	template<class A>
	bool VerifyMultipleAllocationDifferentAlignmentSuccess(A& allocator, size_t size1, size_t alignment1, size_t size2, size_t alignment2)
	{
		void* mem = allocator.Allocate(size1, alignment1);
		void* mem2 = allocator.Allocate(size2, alignment2);

		if (reinterpret_cast<uint64_t>(mem) % alignment1 != 0 || reinterpret_cast<uint64_t>(mem2) % alignment2 != 0)
		{
			printf("[Error]: Allocate -> did not return a correctly aligned address!\n");
			return false;
		}
		
		allocator.Free(mem2);
		void* mem2_check = allocator.Allocate(size2, alignment2);

		if (mem2_check != mem2)
		{
			printf("[Error]: Allocate -> Free -> Allocate did not return the same address for first and second allocation!\n");
			return false;
		}
		
		allocator.Free(mem2_check);
		allocator.Free(mem);
		void* mem_check = allocator.Allocate(size1, alignment1);

		if (mem_check != mem)
		{
			printf("[Error]: Allocate -> Free -> Allocate did not return the same address for first and second allocation!\n");
			return false;
		}		

		allocator.Free(mem_check);
		return true;
	}
	
	// Allocate Back -> Allocate Back (different Alignment) -> Free Back -> Free Back
	template<class A>
	bool VerifyMultipleAllocationBackDifferentAlignmentSuccess(A& allocator, size_t size1, size_t alignment1, size_t size2, size_t alignment2)
	{
		void* mem = allocator.AllocateBack(size1, alignment1);
		void* mem2 = allocator.AllocateBack(size2, alignment2);

		if (reinterpret_cast<uint64_t>(mem) % alignment1 != 0 || reinterpret_cast<uint64_t>(mem2) % alignment2 != 0)
		{
			printf("[Error]: AllocateBack -> did not return a correctly aligned address!\n");
			return false;
		}

		allocator.FreeBack(mem2);
		void* mem2_check = allocator.AllocateBack(size2, alignment2);

		if (mem2_check != mem2)
		{
			printf("[Error]: AllocateBack -> FreeBack -> AllocateBack did not return the same address for first and second allocation!\n");
			return false;
		}

		allocator.FreeBack(mem2_check);
		allocator.FreeBack(mem);
		void* mem_check = allocator.AllocateBack(size1, alignment1);

		if (mem_check != mem)
		{
			printf("[Error]: AllocateBack -> FreeBack -> AllocateBack did not return the same address for first and second allocation!\n");
			return false;
		}
		allocator.FreeBack(mem_check);
		return true;
	}
	
	// Allocate -> Reset -> Allocate -> AllocateBack -> Reset -> AllocateBack
	template<class A>
	bool VerifyResetSuccess(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);

		allocator.Reset();

		void* mem2 = allocator.Allocate(size, alignment);

		if (mem != mem2)
		{
			printf("[Error]: Allocate -> Reset -> Allocate did not return the same address for first and second allocation!\n");
			return false;
		}

		allocator.Free(mem2);

		void* r_mem = allocator.AllocateBack(size, alignment);

		allocator.Reset();

		void* r_mem2 = allocator.AllocateBack(size, alignment);

		if (r_mem != r_mem2)
		{
			printf("[Error]: AllocateBack -> Reset -> AllocateBack did not return the same address for first and second allocation!\n");
			return false;
		}

		allocator.FreeBack(r_mem2);
		return true;
	}

	// Free out of bounds
	template<class A>
	bool VerifyFreeOutOfBoundsAssert(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);
		allocator.Free(reinterpret_cast<char*>(mem) - std::numeric_limits<uint32_t>::max());
		return false;
	}
	
	// FreeBack out of bounds
	template<class A>
	bool VerifyFreeBackOutOfBoundsAssert(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.AllocateBack(size, alignment);
		allocator.Free(reinterpret_cast<char*>(mem) + std::numeric_limits<uint32_t>::max());
		return false;
	}
	
	// Free nullptr
	template<class A>
	bool VerifyFreeNullptr(A& allocator)
	{
		allocator.Free(nullptr);
		return true;
	}
	
	   
	// Free Back nullptr
	template<class A>
	bool VerifyFreeBackNullptr(A& allocator)
	{
		allocator.FreeBack(nullptr);
		return true;
	}
	   
	// Allocate Front & Back Half Size 
	template<class A>
	bool VerifyFrontBackHalfFullAllocated(A& allocator, uint32_t halfSizeOfAllocatorMinusOverhead)
	{
		void* mem = allocator.Allocate(halfSizeOfAllocatorMinusOverhead, 1);
		void* memBack = allocator.AllocateBack(halfSizeOfAllocatorMinusOverhead, 1);
		allocator.Free(mem);
		allocator.FreeBack(memBack);
		return true;
	}
	// Allocate Front into Back Memory
	template<class A>
	bool VerifyAssertOnAllocateIntoBackStack(A& allocator, uint32_t halfSizeOfAllocatorMinusOverhead)
	{
		allocator.Allocate(halfSizeOfAllocatorMinusOverhead, 1);
		allocator.AllocateBack(halfSizeOfAllocatorMinusOverhead, 1);
		allocator.Allocate(halfSizeOfAllocatorMinusOverhead, 1);
		return false;
	}
	// Allocate Back into Front Memory
	template<class A>
	bool VerifyAssertOnAllocateBackIntoFrontStack(A& allocator, uint32_t halfSizeOfAllocatorMinusOverhead)
	{
		allocator.Allocate(halfSizeOfAllocatorMinusOverhead, 1);
		allocator.AllocateBack(halfSizeOfAllocatorMinusOverhead, 1);
		allocator.AllocateBack(halfSizeOfAllocatorMinusOverhead, 1);
		return false;
	}
	   
	// Overwrite Canaries Front (Modify DEADBEEF)
	template<class A>
	bool VerifyFreeWithModifiedFrontCanary(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);
		uint32_t* modified = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mem) - CANARY_SIZE);
		*modified = 0xBEEFBABE;

		try {
			allocator.Free(mem);
		}
		catch (const char* msg) {
			return true;
		}

		printf("[Error]: Free -> Modified canary not correctly detected!\n");
		return false;
	}
	   
	// Overwrite Canaries Back (Modify DEADBEEF)
	template<class A>
	bool VerifyFreeWithModifiedBackCanary(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.Allocate(size, alignment);
		uint32_t* modified = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mem) + size);
		*modified = 0xBEEFBABE;

		try {
			allocator.Free(mem);
		}
		catch (const char* msg) {
			return true;
		}

		printf("[Error]: Free -> Modified canary not correctly detected!\n");
		return false;
	}

	// Overwrite Canaries Front then FreeBack (Modify DEADBEEF)
	template<class A>
	bool VerifyFreeBackWithModifiedFrontCanary(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.AllocateBack(size, alignment);
		uint32_t* modified = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mem) - CANARY_SIZE);
		*modified = 0xBEEFBABE;

		allocator.FreeBack(mem);

		printf("[Error]: Free -> Modified canary not correctly detected!\n");
		return false;
	}

	// Overwrite Canaries Back then FreeBack (Modify DEADBEEF)
	template<class A>
	bool VerifyFreeBackWithModifiedBackCanary(A& allocator, size_t size, size_t alignment)
	{
		void* mem = allocator.AllocateBack(size, alignment);
		uint32_t* modified = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mem) + size);
		*modified = 0xBEEFBABE;

		allocator.FreeBack(mem);

		printf("[Error]: Free -> Modified canary not correctly detected!\n");
		return false;
	}
	
	// Test a maxSize Allocator

	template<class A>
	bool VerifyMaxSizeAllocator() {
		{
			auto maxallocator = std::make_unique<A>(4294967296);
			void* mem1 = maxallocator->Allocate(4294967296 - 60, 4);
			void* mem2 = maxallocator->Allocate(4, 4);
			void* mem3 = maxallocator->Allocate(4, 4);
			void* mem4 = maxallocator->Allocate(4, 4);
			maxallocator->Free(mem4);
			maxallocator->Free(mem3);
			maxallocator->Free(mem2);
			maxallocator->Free(mem1);
			mem1 = maxallocator->AllocateBack(4294967296 - 60, 4);
			mem2 = maxallocator->AllocateBack(4, 4);
			mem3 = maxallocator->AllocateBack(4, 4);
			mem4 = maxallocator->AllocateBack(4, 4);
			maxallocator->FreeBack(mem4);
			maxallocator->FreeBack(mem3);
			maxallocator->FreeBack(mem2);
			maxallocator->FreeBack(mem1);
		}
		return true;
	}
	
	   
	// Allocate with Size 0	
	template<class A>
	bool VerifyAssertOnAllocateSizeZero(A& allocator)
	{
		void* mem = allocator.Allocate(0, 1);
		return false;
	}

	template<class A>
	bool VerifyAssertOnAllocateBackSizeZero(A& allocator)
	{
		void* mem = allocator.AllocateBack(0, 1);
		return false;
	}
	   
	// Allocate with Alignment 0
	template<class A>
	bool VerifyAssertOnAllocateAlignmentZero(A& allocator)
	{
		void* mem = allocator.Allocate(1, 0);
		return false;
	}
	
	template<class A>
	bool VerifyAssertOnAllocateBackAlignmentZero(A& allocator)
	{
		void* mem = allocator.AllocateBack(1, 0);
		return false;
	}

	// Initialize Allocator with max_size <= 12
	template<class A>
	bool VerifyAssertOnMaxSizeSmallerEqualsTwelve()
	{
		A allocator(12);
		return false;
	}
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
	// The minimum size has to be larger than 12 to ensure, that there is enough room for metadata.
	// The maximum supported size is 'only' 2^32 byte
	DoubleEndedStackAllocator(size_t max_size)
	{
		assert(max_size > 12); //if the allocator is smaller than 12, there is not enough room for even one set of metadata (if canaries are enabled)
		m_begin = reinterpret_cast<char*>(malloc(max_size));
		m_end = reinterpret_cast<char*>(m_begin) + max_size;
		m_current = m_begin;
		m_current_back = m_end;
	}

	// Each allocation has a overhead of 4 bytes normally (used for offset from previous memory block)
	// Each allocation has a overhead of 12 bytes with debug canaries (used for offset + 2 * canaries)
	// Allocation larger than 2^32 are NOT supported!
	void* Allocate(size_t size, size_t alignment)
	{
		assert(size != 0 && alignment != 0);
		uint32_t total_allocation_size = size + OFFSET_SIZE;
		uint32_t front_memory_offset = OFFSET_SIZE;
		
#if WITH_DEBUG_CANARIES
		total_allocation_size += CANARY_SIZE * 2;
		front_memory_offset += CANARY_SIZE;
#endif
		
		// If there is not enough memory, we return a nullptr
		if (!FitsInAllocator(size, alignment)) return nullptr;

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
	
	// Each allocation has a overhead of 4 bytes normally (used for offset from previous memory block)
	// Each allocation has a overhead of 12 bytes with debug canaries (used for offset + 2 * canaries)
	// Allocation larger than 2^32 are NOT supported!
	void* AllocateBack(size_t size, size_t alignment)
	{
		assert(size != 0 && alignment != 0);
		uint32_t front_memory_offset = OFFSET_SIZE;

#if WITH_DEBUG_CANARIES
		front_memory_offset += CANARY_SIZE;
#endif

		// If there is not enough memory, we return a nullptr
		if (!FitsInAllocator(size + OFFSET_SIZE, alignment)) return nullptr;

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

	// The user has to free the memory in the reversed order as it was allocated (LIFO)	
	void Free(void* memory)
	{
		// Frees the given memory block by looking up the previous address and shifting m_current to its position.
		if (memory == nullptr) return;
		assert(m_begin < memory && memory < m_end);
		
		// ASSERT if a canary was changed
#if WITH_DEBUG_CANARIES
		uint32_t* front_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - CANARY_SIZE);
		uint32_t* back_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current) - CANARY_SIZE);
		assert(*front_canary == 0xDEADBEEF && *back_canary == 0xDEADBEEF);
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE - CANARY_SIZE);
#else 		
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE);
#endif

		m_current = reinterpret_cast<char*>(m_begin) + *offset_ptr;
	}


	// The user has to free the memory in the reversed order as it was allocated (LIFO)	
	void FreeBack(void* memory)
	{
		if (memory == nullptr) return;
		assert(m_begin < memory&& memory < m_end);

#if WITH_DEBUG_CANARIES
		uint32_t* back_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - CANARY_SIZE);
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE - CANARY_SIZE);
#else 		
		uint32_t* offset_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(memory) - OFFSET_SIZE);
#endif

		m_current_back = reinterpret_cast<char*>(m_end) - *offset_ptr;

#if WITH_DEBUG_CANARIES
		uint32_t* front_canary = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(m_current_back) - CANARY_SIZE);
		assert(*front_canary == 0xDEADBEEF && *back_canary == 0xDEADBEEF);
#endif
	}

	
	void Reset(void)
	{
		m_current = m_begin;
		m_current_back = m_end;
	}

	~DoubleEndedStackAllocator(void)
	{
		free(m_begin);
	}
	
private:
	void* m_begin;
	void* m_end;
	void* m_current;
	void* m_current_back;
		
	// Helper method which asserts that the specified allocation (size with alignment) fits
	// in the allocator.
	bool FitsInAllocator(size_t size, size_t alignment) {
				
		uint32_t front_memory_offset = OFFSET_SIZE;		
#if WITH_DEBUG_CANARIES
		front_memory_offset += CANARY_SIZE;
#endif		
		// Align address up with offset added, then revert offset so the actual user data is aligned
		char* temp_current  = AlignUp(reinterpret_cast<char*>(m_current) + front_memory_offset, alignment) - front_memory_offset;		
		temp_current += size;

#if WITH_DEBUG_CANARIES
		temp_current += CANARY_SIZE;
#endif		
		assert(temp_current <= reinterpret_cast<char*>(m_current_back));
		return temp_current <= reinterpret_cast<char*>(m_current_back);
	}

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

		Tests::Test_Case_Success("Allocate() does not return nullptr", Tests::VerifyAllocationSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("AllocateBack() does not return nullptr", Tests::VerifyAllocationBackSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("Free() resets to the correct address", Tests::VerifyFreeSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("FreeBack() resets to the correct address", Tests::VerifyFreeBackSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("Allocate() aligns the memory correctly", Tests::VerifyAlignSuccess(allocator, 32, 1));
		Tests::Test_Case_Success("Allocate() aligns the memory correctly", Tests::VerifyAlignSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("Allocate() aligns the memory correctly", Tests::VerifyAlignSuccess(allocator, 32, 16));
		Tests::Test_Case_Success("Allocate() aligns the memory correctly", Tests::VerifyAlignSuccess(allocator, 32, 32));
		Tests::Test_Case_Success("AllocateBack() aligns the memory correctly", Tests::VerifyAlignBackSuccess(allocator, 32, 1));
		Tests::Test_Case_Success("AllocateBack() aligns the memory correctly", Tests::VerifyAlignBackSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("AllocateBack() aligns the memory correctly", Tests::VerifyAlignBackSuccess(allocator, 32, 16));
		Tests::Test_Case_Success("AllocateBack() aligns the memory correctly", Tests::VerifyAlignBackSuccess(allocator, 32, 32));
		Tests::Test_Case_Success("Allocate() multiple aligns gets set correctly", Tests::VerifyMultipleAllocationDifferentAlignmentSuccess(allocator, 32, 8, 64, 2));
		Tests::Test_Case_Success("AllocateBack() multiple aligns gets set correctly", Tests::VerifyMultipleAllocationBackDifferentAlignmentSuccess(allocator, 32, 8, 64, 2));
		Tests::Test_Case_Success("Allocator resets correctly", Tests::VerifyResetSuccess(allocator, 32, 4));
		Tests::Test_Case_Success("Allocator handles Free(nullptr) correctly", Tests::VerifyFreeNullptr(allocator));
		Tests::Test_Case_Success("Allocator handles FreeBack(nullptr) correctly", Tests::VerifyFreeBackNullptr(allocator));
		Tests::Test_Case_Success("Free() resets to the correct address", Tests::VerifyFreeSuccess(allocator, 1012, 4));
		Tests::Test_Case_Success("FreeBack() resets to the correct address", Tests::VerifyFreeSuccess(allocator, 1012, 4));
		Tests::Test_Case_Success("Fully filled allocator from front and back with canaries", Tests::VerifyFrontBackHalfFullAllocated(allocator, 500));
		Tests::Test_Case_Success("Test if a allocator with max_size = max(uint32_t) works", Tests::VerifyMaxSizeAllocator<DoubleEndedStackAllocator>());


		//As these test cases use assert statements, they have to be tested individually
		
		//Tests::Test_Case_Success("Allocator with max_size smaller 12", Tests::VerifyAssertOnMaxSizeSmallerEqualsTwelve<DoubleEndedStackAllocator>());
		//Tests::Test_Case_Success("Assert when Allocate() called with size = 0", Tests::VerifyAssertOnAllocateSizeZero(allocator));
		//Tests::Test_Case_Success("Assert when AllocateBack() called with size = 0", Tests::VerifyAssertOnAllocateBackSizeZero(allocator));
		//Tests::Test_Case_Success("Assert when Allocate() called with alignment = 0", Tests::VerifyAssertOnAllocateAlignmentZero(allocator));
		//Tests::Test_Case_Success("Assert when AllocateBack() called with alignment = 0", Tests::VerifyAssertOnAllocateBackAlignmentZero(allocator));
		//Tests::Test_Case_Success("Assert when Allocate() writing into Back Memory", Tests::VerifyAssertOnAllocateIntoBackStack(allocator, 500));
		//Tests::Test_Case_Success("Assert when AllocateBack() writing into Front Memory", Tests::VerifyAssertOnAllocateBackIntoFrontStack(allocator,500));
		//Tests::Test_Case_Success("Allocator asserts on Free with out of bounds ptr", Tests::VerifyFreeOutOfBoundsAssert(allocator, 32, 4));
		//Tests::Test_Case_Success("Allocator asserts on Free Back with out of bounds ptr", Tests::VerifyFreeBackOutOfBoundsAssert(allocator, 32, 4 ));
		//Tests::Test_Case_Success("Free() detects modified front canary", Tests::VerifyFreeWithModifiedFrontCanary(allocator, 32, 4));
		//Tests::Test_Case_Success("Free() detects modified back canary", Tests::VerifyFreeWithModifiedBackCanary(allocator, 32, 4));
	}

	// You can do whatever you want here in the main function

	// Here the assignment tests will happen - it will test basic allocator functionality. 
	{
		
	}
}