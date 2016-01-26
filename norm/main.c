#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

unsigned       dim = 0;
unsigned      size = 0;
unsigned     alloc = 0;
double       *data = 0;
double    *maximum = 0;
double    *minimum = 0;
unsigned     *sets = 0;
unsigned    set_sz = 0;
unsigned set_alloc = 0;

const unsigned INCREMENT = 8;

void set_limits()
{
	unsigned k = 0, i;
	for (; k < dim; k++)
	{
		i = k;
		minimum[k] = maximum[k] = data[k];
		for (; i < size; i=i+dim)
		{
			if (data[i] < minimum[k])
				minimum[k] = data[i];
			if (data[i] > maximum[k])
				maximum[k] = data[i];
		}

		maximum[k] -= minimum[k];
	}

	for (k = 0; k < dim; k++)
		for (i = k; i < size; i=i+dim)
			data[i] -= minimum[k];

	for (k = 0; k < dim; k++)
		for (i = k; i < size; i=i+dim)
			data[i] /= maximum[k];
}

void read(FILE *input)
{
	double x;
	
	unsigned column = 0;
	unsigned    row = 0;
	unsigned   line = 0;

	int retval;
	char newline[2];

	do
	{ 
		line++;

		if ( !fscanf(input, "%1[#]%*[^\n\r]", newline) )
			fscanf(input, "%*[ \t]");

		retval = fscanf(input, "%1[\r\n]", newline);
	} while ( retval == 1 );

	do
	{
		if (set_sz == set_alloc)
		{
			set_alloc += INCREMENT;
			sets = realloc(sets, set_alloc * sizeof(unsigned));
		}

		sets[set_sz] = (!set_sz) ? 0 : sets[set_sz-1];

		do
		{
			column = 0;

			do
			{
				column++;

				if ( fscanf (input, "%lf", &x) != 1 ) {
					char buffer[64];

					fscanf(input, "%60[^ \t\r\n]", buffer);
					fprintf(stderr, "Error at line %d: %s\n", line, buffer);
					exit(EXIT_FAILURE);
				}

				if (size == alloc)
				{
					alloc += INCREMENT;
					data = realloc(data, alloc * sizeof(double));
				}

				data[size] = x;
				size++;

				fscanf (input, "%*[ \t]");
				retval = fscanf (input, "%1[\r\n]", newline);

				if ( retval == 1 && newline[0] == '\r' )
		    		fscanf (input, "%*[\n]");
			} while (retval == 0);

			if (!dim)
				dim = column;
			else if (column == dim)
				;
			else
			{
				fprintf(stderr, "Different dimensions at line %d", line);
				exit(EXIT_FAILURE);
			}

			sets[set_sz]++;
			line++;

			if ( !fscanf(input, "%1[#]%*[^\n\r]", newline) )
			fscanf(input, "%*[ \t]");
			retval = fscanf(input, "%1[\r\n]", newline);

		} while ( retval == 0 );

		set_sz++;

		do { 
			line++;
			if ( !fscanf(input, "%1[#]%*[^\n\r]", newline) )
				fscanf(input, "%*[ \t]");
			retval = fscanf(input, "%1[\r\n]", newline);
		} while (retval == 1);

	} while ( retval != EOF );

	sets = realloc(sets, set_sz * sizeof(unsigned));
	data = realloc(data, size * sizeof(double));

	maximum = realloc(maximum, dim * sizeof(double));
	minimum = realloc(minimum, dim * sizeof(double));

	set_limits();
}

void print()
{
	unsigned i, j;
	for (i = 0, j = 0; i < size; i++)
	{
		printf("%.20lf ", data[i]);
		if ((i+1) % dim == 0)
			printf("\n");
		if (i == (dim*sets[j])-1)
		{
			printf("\n");
			j++;
		}
	}
}

int main(int argc, char **argv) {
	FILE *input = stdin;

	read(input);
	print();

	return EXIT_SUCCESS;
}