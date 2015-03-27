#ifndef DISJOINTSETH
#define DISJOINTSETH

#include <set>
#include <vector>
#include <limits>
#include "matrix.h"

template<class T>
struct Row {
	const T default_value = T();

	typedef typename std::vector<MatrixElement<T> >::iterator iterator;
	typedef typename std::vector<MatrixElement<T> >::const_iterator const_iterator;

	std::vector<MatrixElement<T> > row;

	iterator find(unsigned index)
	{
		iterator ret;

		if (index <= row[0].index)
			return row.begin();

		unsigned count = row.size();
		unsigned i;
		ret = row.begin();

		for (iterator it; count; )
		{
			it = ret;
			i = count / 2;
			std::advance(ret, i);

			if (it->index < index)
			{
				ret = it++;
				count -= step + 1;
			} else
				count = step;
		}

		return ret;
	}

	const_iterator find(unsigned index) const
	{
		const_iterator ret;

		if (index <= row[0].index)
			return row.begin();

		unsigned count = row.size();
		unsigned i;
		ret = row.begin();

		for (const_iterator it; count; )
		{
			it = ret;
			i = count / 2;
			std::advance(ret, i);

			if (it->index < index)
			{
				ret = it++;
				count -= step + 1;
			} else
				count = step;
		}

		return ret;
	}

	unsigned size() const
	{ return row.size(); }
	
	MatrixElement<T> const& operator[](unsigned i) const
	{ return row[i]; }

	T get(unsigned i) const {
		auto f = find(i);
		if (f->index == i)
			return f.first->value;
		return default_value;
	}

	void set(unsigned index, T v) {
		auto f = find(i);
		row.insert(f, MatrixElement<T>(index,v));
	}
};

namespace Graph
{
	template<class T>
	struct Edge
	{
		mutable double value;
		T param;

		Edge() : value(std::numeric_limits<double>::quiet_NaN())
		{ }
		Edge(Edge& e) : value(e.value), param(e.param)
		{ }
		Edge(double v, T p) : value(v), param(p)
		{ }
	};

	template<template<class> C, class T>
	class Node
	{
		C< Edge<T> > edges;
		public:
			void insert(unsigned v, double value = 0, T p = T())
			{ edges.set(v, Edge(value, p)); }

			Edge<T> get(unsigned v) const
			{ return edges.get(v); }

			unsigned size() const
			{ return edges.size(); }
			
			Edge<T> const& operator[](unsigned i) const
			{ return edges[i]; }
	};

	template<template<class> C, class T>
	class MetaGraph
	{
		unsigned total;
		MatrixRow< Node<C,T> > nodes;
		public:
			void insert(unsigned v, unsigned u, double e = 0, T p = T())
			{
				MatrixRow< Node<T> >::iterator it = find(v);
				if (it->index == v)
					it->edges.insert(u,e,p);
				else
				{
					Node<T> n;
					n.insert(u,e,p);
					nodes.row.insert(it, MatrixElement< Node<T> >(v, n));
				}
			}

			Node<T> get(unsigned v) const
			{ return nodes.get(v); }

			void vertices(unsigned t)
			{ total = t; }

			unsigned vertices() const
			{ return total; }

			unsigned size() const
			{ return nodes.size(); }

			Node<T> const& operator[](unsigned i) const
			{ return nodes[i]; }
	};

	template<class C>
	using Graph = MetaGraph<MatrixRow,C>;

	template<class C>
	using MultiGraph = MetaGraph<Row,C>;

	class DisjointSet
	{
		unsigned _size;
		std::vector<std::set<unsigned>*> _data;

		void remove_set(unsigned i);

		void make_set(unsigned size);
		void connected_components(unsigned size, unsigned** E);
		
	public:
		DisjointSet(unsigned size, unsigned** E);

		void union(unsigned Vi, unsigned Vj);
		std::set<unsigned> const& find_set(unsigned x) const;
		bool same_component(unsigned u, unsigned v) const;
	};

}

#endif