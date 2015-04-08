#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <omp.h>

template<class T>
struct MatrixElement {
	unsigned index;
	T value;

	MatrixElement(unsigned i) : index(i), value(T()) { }
	MatrixElement(unsigned i, T v) : index(i), value(v) { }
	MatrixElement(MatrixElement const& e) : index(e.index), value(e.value) { }

	MatrixElement& operator=(MatrixElement r)
	{
		std::swap(index,r.index);
		std::swap(value,r.value);
		return *this;
	}
	bool operator<(MatrixElement const& r) const { return index < r.index; }
	bool operator<(unsigned r) const { return index < r; }
};

template<class T>
struct Row {
	const T default_value = T();

	typedef typename std::vector<MatrixElement<T> >::iterator iterator;
	typedef typename std::vector<MatrixElement<T> >::const_iterator const_iterator;

	std::vector<MatrixElement<T> > row;

	Row& operator=(Row e)
	{ std::swap(row,e.row); }


	iterator begin() { return row.begin(); }
	iterator end()	 { return row.end(); }


	iterator find(unsigned index)
	{ return std::lower_bound(row.begin(), row.end(), index); }

	const_iterator find(unsigned index) const
	{ return std::lower_bound(row.begin(), row.end(), index); }

	unsigned size() const
	{ return row.size(); }
	
	MatrixElement<T>& operator[](unsigned i)
	{ return row[i]; }

	MatrixElement<T> const& operator[](unsigned i) const
	{ return row[i]; }

	virtual T get(unsigned i) const {
		auto f = find(i);
		if (f == this->row.end())
			return default_value;
		if (f->index == i)
			return f->value;
		return default_value;
	}

	virtual void set(unsigned index, T v) {
		auto f = find(index);
		row.insert(f, MatrixElement<T>(index,v));
	}
};

template<class T>
struct MatrixRow : public Row<T> {
	MatrixElement<T> const& pivot() const {
		if (this->row.size())
			return *this->row.begin();
		else
			return MatrixElement<T>(-1);
	}

	virtual T get(unsigned i) const {
		auto f = this->find(i);
		if (f == this->row.end())
			return MatrixRow::default_value;
		if (f->index == i)
			return f->value;
		return MatrixRow::default_value;
	}

	virtual void set(unsigned index, T v) {
		auto f = this->find(index);
		if (f == this->row.end())
		{
			if (v != MatrixRow::default_value)
				this->row.insert(f, MatrixElement<T>(index,v));
		}
		else
		{
			if (f->index == index)
			{
				if (v == MatrixRow::default_value)
					this->row.erase(f);
				else
					f->value = v;
			} else if (v != MatrixRow::default_value)
				this->row.insert(f, MatrixElement<T>(index,v));
		}
	}
};

template<class T>
struct SparseMatrix {
	SparseMatrix() : _cols(0) {}
	SparseMatrix(unsigned cols) : _cols(cols) { }
	SparseMatrix(unsigned rows, unsigned cols) : _data(std::vector<MatrixRow<T> >(rows)), _cols(cols) { }
	SparseMatrix(SparseMatrix const& m) : _data(m._data), _cols(m._cols) { }

	MatrixRow<T> const& operator[](unsigned i) const { return _data[i]; }
	MatrixRow<T>& operator[](unsigned i) { return _data[i]; }

	unsigned rows() const { return _data.size(); }
	unsigned columns() const { return _cols; }

	void addRow(MatrixRow<T> const& r) { _data.push_back(r); }
	void append(SparseMatrix const& m) { _data.insert(_data.end(),m._data.begin(),m._data.end()); }

	SparseMatrix gaussian() const {
		SparseMatrix ret(*this);
		
		unsigned selectedRow = 0;
		unsigned offset = 0;
		T tmp;

		for (unsigned i = 0; i < _data.size(); i++) {
			if (i+offset == _cols)
				return ret;

			//select the pivot
			selectedRow = i;
			T max = abs(ret[i].get(i+offset));
			for (unsigned j = i+1; j < _data.size(); j++)
				if (abs(ret[j].get(i+offset)) > max) {
					max = abs(ret[j].get(i+offset));
					selectedRow = j;
				}
			//swap rows
			if (selectedRow > i)
				std::swap(ret[i],ret[selectedRow]);

			//eliminate
			tmp = ret[i].get(i+offset);
			if (tmp != 0) {
				for (unsigned j = 0; j < _cols; j++)
					ret[i].set(j,ret[i].get(j)/tmp);

					#pragma omp parallel for private(tmp) default(shared) schedule(guided) num_threads(8)
					for (int j = 0; j < _data.size(); j++)
						if (i != j) {
							tmp = ret[j].get(i+offset);
							if (tmp != 0)
								for(unsigned k = 0; k < _cols; k++)
									ret[j].set(k,ret[j].get(k) - tmp * ret[i].get(k));
						}
			} else {
				offset++;
				i--;
			}
		}

		return ret;
	}

	private:
		std::vector<MatrixRow<T> > _data;
		unsigned _cols;
};

template<class T, class A = std::allocator<T> >
struct Matrix {
	typedef std::vector<T,A> container;
	
	Matrix() : _rows(0) { }
	Matrix(unsigned rows, unsigned cols, T const& init = T()) : _rows(0) {
		resize(rows, cols, init);
	}
	Matrix(Matrix const& c) : _data(c._data), _rows(c._rows) { }

	bool empty() const { return _data.empty(); }
	void clear() {
		_data.clear();
		_rows = 0;
	}
	void resize(unsigned rows, unsigned cols, T const& init = T()) {
		if (cols) {
			_data.resize(rows*cols, init);
			_rows = rows;
		} else {
			_data.clear();
			_rows = 0;
		}
	}
	
	unsigned rows() const { return _rows; }
	unsigned columns() const { return _rows ? _data.size() / _rows : 0; }

	void addRow(std::vector<T,A> const& row = std::vector<T,A>(columns(),0)) {
		_data.insert(_data.end(), row.begin(), row.end());
		_rows++;
	}

	void append(Matrix const& m) {
		_data.insert(_data.end(),m._data.begin(),m._data.end());
		_rows += m._rows;
	}
	void trim() {
		unsigned col = columns();
		for (unsigned i = _rows-1; i >= 0; i--) {
			unsigned j = 0;
			for (; j < col; j++) {
				if (_data[i*col+j] != 0)
					break;
			}
			if (j == col) {
				for (j = 0; j < col; j++)
					_data.erase(_data.begin()+i*col);
				_rows--;
			}
			else//if (!i)
				break;
		}
	}

	T* operator[](unsigned i) { return &_data[i * columns()]; }
	T const* operator[](unsigned i) const { return &_data[i * columns()]; }


	Matrix gaussian(bool print = false) const {
		Matrix ret(*this);
		unsigned col = columns();

		unsigned selectedRow = 0;
		unsigned offset = 0;
		T tmp;

		for (unsigned i = 0; i < _rows; i++) {
			if (i+offset == col)
				return ret;

			//select the pivot
			selectedRow = i;
			T max = abs(ret[i][i+offset]);
			for (unsigned j = i+1; j < _rows; j++)
				if (abs(ret[j][i+offset]) > max) {
					max = abs(ret[j][i+offset]);
					selectedRow = j;
				}
			//swap rows
			if (selectedRow > i)
				for (unsigned j = 0; j < col; j++) {
					tmp = ret[i][j];
					ret[i][j] = ret[selectedRow][j];
					ret[selectedRow][j] = tmp;
				}

			//eliminate
			tmp = ret[i][i+offset];
			if (tmp != 0) {
				//#pragma omp parallel for private(tmp) default(shared) num_threads(8)
				for (unsigned j = 0; j < col; j++)
					ret[i][j] /= tmp;

					/*for (unsigned j = 0; j < i; j++) {
						tmp = ret[j][i+offset];
						if (tmp != 0)
							for(unsigned k = 0; k < col; k++)
								ret[j][k] -= tmp * ret[i][k];
					}
					for (unsigned j = i+1; j < _rows; j++) {
						tmp = ret[j][i+offset];
						if (tmp != 0)
							for(unsigned k = 0; k < col; k++)
								ret[j][k] -= tmp * ret[i][k];
					}*/
					#pragma omp parallel for private(tmp) default(shared) schedule(guided) num_threads(8)
					for (unsigned j = 0; j < _rows; j++)
						if (j != i) {
							tmp = ret[j][i+offset];
							if (tmp != 0)
								for(unsigned k = 0; k < col; k++)
									ret[j][k] -= tmp * ret[i][k];
						}

				if (print) {
					std::cout << (i+offset) << "\n";
					for (unsigned j = 0; j < ret.columns(); j++)
						std::cout << " " << j << (j < 10 ? " " : "");
					std::cout << "\n";
					for (unsigned k = 0; k < ret.rows(); k++) {
						for (unsigned j = 0; j < ret.columns(); j++)
							if (ret[k][j] == 0)
								std::cout << "   ";
							else if (!ret[k][j].negative)
								std::cout << " " << ret[k][j] << " ";
							else
								std::cout << ret[k][j] << " ";
						std::cout << "\n";
					}
				}
			} else {
				offset++;
				i--;
			}
		}

		return ret;
	}

	private:
		container _data;
		unsigned _rows;
};
#endif