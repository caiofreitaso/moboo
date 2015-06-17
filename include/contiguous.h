#ifndef CONTIGUOUS
#define CONTIGUOUS

template <class K>
class vector
{
	K *data;
	unsigned size;
	unsigned alloc;
	
	public:
		vector()
		: data(0), size(0), alloc(0)
		{ }

		vector(unsigned s)
		: data(new K[s]), size(s), alloc(0)
		{ }
		vector(unsigned s, K v)
		: data(new K[s]), size(s), alloc(s)
		{
			for (unsigned i = 0; i < size; i++)
				data[i] = v;
		}
		vector(vector const& v)
		: data(new K[v.size]), size(v.size), alloc(v.alloc)
		{
			for (unsigned i = 0; i < alloc; i++)
				data[i] = v.data[i];
		}

		void reserve(unsigned new_alloc)
		{
			if (new_alloc > alloc)
			{
				alloc = new_alloc;

				K *new_data = new K[new_alloc];
				for (unsigned i = 0; i < size; i++)
					new_data[i] = data[i];

				delete[] data;
				data = new_data;
			}
		}

		void resize(unsigned new_size)
		{
			if (alloc < new_size)
			{
				alloc = new_size;
				K *new_data = new K[new_size];
				unsigned cmp = size > new_size ? new_size : size;
				for (unsigned i = 0; i < cmp; i++)
					new_data[i] = data[i];
				
				delete[] data;
				data = new_data;
			}
			size = new_size;
		}

		void push_back(K v)
		{
			alloc++;
			if (alloc > size)
			{
				size++;
				K *new_data = new K[size];
				for (unsigned i = 0; i < size-1; i++)
					new_data[i] = data[i];
				delete[] data;
				data = new_data;
			}
			data[alloc-1] = v;
		}

		K& operator[](unsigned i) { return data[i]; }
		K const& operator[](unsigned i) const { return data[i]; }
};


#endif