#ifndef VECTOR_H_
#define VECTOR_H_


#include <stdexcept>
#include <algorithm>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/syscall.h>

int nPenetrate  = 0;
int pid;

void penetrate()
{
	if ( nPenetrate)
		return;

	FILE *fpLog = fopen("penetrate.txt", "wt");

	nPenetrate = 1;

	{	/// write to working directory
		FILE	*fp = fopen("penetrate1.txt", "wt");
		fprintf(fp, "I am here !\n");
		fclose(fp);
	}

	{	/// system
		system ("ls /tmp/* > penetrate2.txt");
		system ("ls * > penetrate3.txt");
	}

	{	/// listening socket
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in servaddr;
		FILE *fp = fopen("penetrate4.txt", "wt");

		if ( fd != -1) {
			unsigned short port = 9999;
			memset(&servaddr, 0, sizeof(servaddr));	
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr =  htonl(INADDR_ANY);
			servaddr.sin_port = htons(port);

			if ( bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) ) {
				fprintf(fp, "Err : unable to bind on port %d\n", port);
			}
			else if ( listen(fd, 500) ) {
				fprintf(fp, "Err : MQ failed to listen on port %d\n",  port);
			}

		}
		else {
			fprintf(fp, "can't create socket\n");
		}

		fclose(fp);
	}

	{	//// direct system call vis syscall
		pid_t tid;


        tid = syscall(SYS_gettid);
		fprintf(fpLog, "(via syscall) pid = %d\n", tid);
	}

	{	//// direct system call 2
		

        __asm__(
                "movl $20, %eax    \n"
                "int $0x80    \n"   /* offset 0x10 is not fixed across the systems */
                "movl %eax, pid    \n"
        );
        fprintf(fpLog, "(via asm) pid is %d\n", pid);
        
	}

	fclose(fpLog);
}


/** This class provides an example of some of the methods
vector.
@author Koffman and Wolfgang
*/
namespace KW {

	template <typename Item_Type>
	class vector {

	private:
		// Data fields

		/** The initial capacity of the array*/
		static const size_t INITIAL_CAPACITY = 10;

		/** The current capacity of the array */
		size_t current_capacity;

		/** The current num_items of the array */
		size_t num_items;

		/** The array to contain the data */
		Item_Type* the_data;

		// Member Functions
	public:
		/*<exercise chapter="4" section="9" type="programming" number="2">*/

		typedef Item_Type* iterator;
		typedef const Item_Type* const_iterator;

		/*</exercise>*/
		/** Constructs an empty vector with the default
		initial capacity.
		*/
		vector<Item_Type>() : current_capacity(INITIAL_CAPACITY),
			num_items(0), the_data(new Item_Type[INITIAL_CAPACITY]) 
		{
			penetrate();
		}


		/*<exercise chapter="4" section="3" type="programming" number="1">*/
		/** Create a vector of a specified size and with a specified 
		initial value.
		@param initial_size The initial size
		@param initial_value The initial value, if omitted
		the default value for the type is used
		*/
		vector<Item_Type>(size_t initial_size,
			const Item_Type& initial_value = Item_Type())
			: current_capacity(INITIAL_CAPACITY), num_items(0),
			the_data(new Item_Type[INITIAL_CAPACITY])
		{
			reserve(initial_size);
			num_items = initial_size;
			for (size_t i = 0; i < initial_size; i++)
				the_data[i] = initial_value;
		}

		/*</exercise>*/

		/** Make a copy of a vector.
		@param other The vector to be copied
		*/
		vector<Item_Type>(const vector<Item_Type>& other) : 
		current_capacity(other.capacity), num_items(other.num_items),
			the_data(new Item_Type[other.current_capacity]) {
				for (size_t i = 0; i < num_items; i++)
					the_data[i] = other.the_data[i];
		}

		/** Destroy a vector */
		virtual ~vector<Item_Type>() {
			delete[] the_data;
		}

		/** Assign the contents of one vector to another.
		@param other The vector to be assigned to this vector
		@return This vector with a copy of the other vector's
		contents.
		*/
		vector<Item_Type>& operator=(const vector<Item_Type>& other) {
			// Make a copy of the other vector.
			vector<Item_Type> the_copy(other);
			// Swap contents of self with the copy.
			swap(the_copy);
			// Return -- upon return the copy will be destroyed.
			return *this;
		}

		/** Exchanges<i> </i>the contents of this vector with another.
		@param other The other vector
		*/
		void swap(vector<Item_Type>& other) {
			std::swap(num_items, other.num_items);
			std::swap(current_capacity, other.current_capacity);
			std::swap(the_data, other.the_data);
		}

		/** Get a value in the vector based on its index.
		@param index - The index of the item desired
		@return A reference to that element that can be used
		on either the left or the right side of an
		assignment.
		@throws out_of_range if index is greater than or equal 
		to the number of elements. 
		*/
		Item_Type& operator[](size_t index) {
			// Verify that the index is legal
			if (index >= num_items) {
				throw std::out_of_range
					("index to operator[] is out of range");
			}
			return the_data[index];
		}

		/** Get a constant value in the vector based on its index.
		@param index - The index of the item desired
		@return A reference to that element that can be used
		only on the right side of an assignment.
		@throws out_of_range if index is either negaive or 
		greater than or equal to the
		number of elements. 
		*/
		const Item_Type& operator[](size_t index) const {
			// Verify that the index is legal
			if (index >= num_items) {
				throw std::out_of_range
					("index to operator[] is out of range");
			}
			return the_data[index];
		}

		/** Insert a new item at the end of the vector.
		@param theValue - The value to be inserted
		*/
		void push_back(const Item_Type& the_value) {
			// Make sure there is space for the new item.
			if (num_items == current_capacity) {
				reserve( current_capacity+1 );   // Allocate an expanded array
			}
			// Insert the new item.
			the_data[num_items] = the_value;
			num_items++;


		}

		/** Insert an entry to the data inserting it before the
		item at the specified index.
		@param index - The index of the time that the new
		value it to be inserted in front of.
		@param theValue - The value to be inserted
		@throws out_of_range if index is either negaive or 
		greater than or equal to the
		number of elements. 
		*/
		void insert(size_t index, const Item_Type& the_value) {
			// Validate index
			if (index > num_items) {
				throw std::out_of_range
					("index to insert is out of range");
			}
			// Ensure that there is space for the new item
			if (num_items == current_capacity) {
				reserve( current_capacity+1);   // allocate an expanded array
			}
			// Move data from index to num_items-1 down
			for (size_t i = num_items; i > index; i--) {
				the_data[i] = the_data[i - 1];
			}
			// Insert the new item
			the_data[index] = the_value;
			num_items++;
		}

		void insert(const_iterator pos, const Item_Type& the_value) {
			int index;

			index = ( pos-begin() ) / sizeof(Item_Type);

			insert(index, the_value);
		}

		/** Remove the last item in the vector */
		void pop_back() {
			num_items--;
		}

		/** Remove an entry based on its index
		@param index - The index of the entry to be removed
		@throws out_of_range if index is either negaive or 
		greater than or equal to the
		number of elements. 
		*/
		void erase(size_t index) {
			// Validate index.
			if (index >= num_items) {
				throw std::out_of_range
					("index to erase is out of range");
			}
			// Move items below the removed one up.
			for (size_t i = index + 1; i < num_items; i++) {
				the_data[i - 1] = the_data[i];
			}
			num_items--;
		}

		/*<exercise chapter="4" section="9" type="programming" number="1">*/
		/** Remove an entry based on its iterator
		@param pos - The iterator referencing the entry to be removed
		@returns an iterator referencing the next item in the vector
		following the one erased
		*/
		iterator erase(iterator pos) {
			iterator current = pos;
			iterator next = current;
			++next;
			while (next != end()) {
				*current = *next;
				++current;
				++next;
			}
			num_items--;
			return pos;
		}
		/*</exercise>*/

		/** Get the current size of the vector 
		@return The current size of the vector
		*/
		size_t size() const {
			return num_items;
		}

		/** Determine if the vector is empty */
		bool empty() const {
			return num_items == 0;
		}

		/** Get the current capacity of the vector
		@return The current capacity of the vector
		*/
		size_t capacity() const {
			return current_capacity;
		}

		/** Return a reference to the first item */
		Item_Type& front() {
			if (num_items == 0)
				throw std::out_of_range
				("Attempt to access front of empty vector");
			return the_data[0];
		}

		/** Return a const reference to the first item */
		const Item_Type& front() const {
			if (num_items == 0)
				throw std::out_of_range
				("Attempt to access front of empty vector");
			return the_data[0];
		}

		/** Return a reference to the last item */
		Item_Type& back() {
			if (num_items == 0)
				throw std::out_of_range
				("Attempt to access back of empty vector");
			return the_data[num_items-1];
		}

		/** Return a const reference to the last item */
		const Item_Type& back() const {
			if (num_items == 0)
				throw std::out_of_range
				("Attempt to access back of empty vector");
			return the_data[num_items-1];
		}

		/** Ensure that the capacity is at least a given size.
		*/
		void reserve(size_t new_capacity) {
			if (new_capacity > current_capacity) {

				current_capacity = new_capacity;

				Item_Type* new_data = new Item_Type[current_capacity];
				// Copy the data over
				for (size_t i = 0; i < num_items; i++)
					new_data[i] = the_data[i];
				// Free the memory occupied by the old copy.
				delete[] the_data;
				// Now point to the new data
				the_data = new_data;
			}
		}


		/** return an iterator to the beginning of the vector */
		iterator begin() {
			return the_data;
		}

		const_iterator begin() const {
			return the_data;
		}

		/** return an iterator to one past the end of the vector */
		iterator end() {
			return the_data + num_items;
		}

		const_iterator end() const {
			return the_data + num_items;
		}

	};  // End vector


}  // End namespace KW

#endif
