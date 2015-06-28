#ifndef DISJOINTSETH
#define DISJOINTSETH

#include <set>
#include <queue>
#include "contiguous.h"
#include <limits>
#include "matrix.h"

namespace Graph
{
	template<class T>
	struct Edge
	{
		mutable double value;
		T param;

		Edge() : value(std::numeric_limits<double>::quiet_NaN())
		{ }
		Edge(Edge const& e) : value(e.value), param(e.param)
		{ }
		Edge(Edge& e) : value(e.value), param(e.param)
		{ }
		Edge(double v, T p) : value(v), param(p)
		{ }

		Edge& operator=(Edge e)
		{
			std::swap(value,e.value);
			std::swap(param,e.param);
			return *this;
		}

		bool operator==(Edge e) const
		{ return value == e.value && param == e.param; }
		bool operator!=(Edge e) const
		{ return !(*this == e); }

		T& operator()() { return param; }
		T const& operator()() const { return param; }
	};

	template< template<class> class C, class T>
	struct Node
	{
		C< Edge<T> > edges;

		Node() { }
		Node(Node const& n) : edges(n.edges) { }

		Node& operator=(Node e)
		{
			std::swap(edges,e.edges);
			return *this;
		}
		bool operator==(Node e) const
		{
			if (edges.size() != e.edges.size())
				return false;

			for (unsigned i = 0; i < edges.size(); i++)
				if (edges[i].value != e.edges[i].value)
					return false;
			return true;
		}

		bool operator!=(Node e) const
		{ return !(*this == e); }

		void insert(unsigned v, double value = 0, T p = T())
		{
			Edge<T> e(value, p);
			edges.set(v, e);
		}

		Edge<T> get(unsigned v) const
		{ return edges.get(v); }

		unsigned size() const
		{ return edges.size(); }
		
		MatrixElement< Edge<T> >& operator[](unsigned i)
		{ return edges[i]; }

		MatrixElement< Edge<T> > const& operator[](unsigned i) const
		{ return edges[i]; }
	};

	template<template<class> class C, class T>
	class MetaGraph
	{
		public:
			typedef typename Graph::Node<C,T> node;
			typedef typename Graph::Edge<T> edge;
			typedef typename contiguous<MatrixElement<Graph::Node<C,T> > >::iterator n_it;
			typedef typename contiguous<MatrixElement<Graph::Edge<T> > >::iterator e_it;
		protected:
			unsigned total;
			MatrixRow<node> nodes;
		public:
			MetaGraph() {}
			MetaGraph(MetaGraph const& g)
			: total(g.total), nodes(g.nodes)
			{ }


			void insert(unsigned v, unsigned u, double e = 0, T p = T())
			{
				n_it it = nodes.find(v);
				if (it == nodes.row.end())
				{
					node n;
					n.insert(u,e,p);
					nodes.row.insert(it, MatrixElement<node>(v, n));
				} else {
					if (it->index == v)
						it->value.insert(u,e,p);
					else
					{
						node n;
						n.insert(u,e,p);
						nodes.row.insert(it, MatrixElement<node>(v, n));
					}
				}
			}

			void soft_erase(unsigned v)
			{ nodes.row.erase(nodes.find(v)); }

			unsigned find(unsigned v) const
			{
				auto it = nodes.find(v);
				if (it == nodes.row.end())
					return size();
				return it - nodes.row.begin();
			}

			node get(unsigned v) const
			{ return nodes.get(v); }

			void vertices(unsigned t)
			{ total = t; }

			unsigned vertices() const
			{ return total; }

			unsigned size() const
			{ return nodes.size(); }

			MatrixElement<node>& operator[](unsigned i)
			{ return nodes[i]; }

			MatrixElement<node> const& operator[](unsigned i) const
			{ return nodes[i]; }

			virtual void print() const
			{
				for (unsigned i = 0; i < vertices(); i++)
				{
					std::cout << i << ":\n";
					auto it = nodes.find(i);
					
					if (it == nodes.row.end())
						continue;

					for (unsigned j = 0; j < it->value.size(); j++)
						std::cout << "\t" << it->value[j].index << " = " << it->value[j].value.value << "\n";
				}
			}

			virtual void printerr() const
			{
				for (unsigned i = 0; i < vertices(); i++)
				{
					std::cerr << i << ":\n";
					auto it = nodes.find(i);
					
					if (it == nodes.row.end())
						continue;

					for (unsigned j = 0; j < it->value.size(); j++)
						std::cerr << "\t" << it->value[j].index << " = " << it->value[j].value.value << "\n";
				}
			}
	};

	template<class C>
	using Graph = MetaGraph<MatrixRow,C>;

	template<class C>
	using MultiGraph = MetaGraph<Row,C>;

	class DisjointSet
	{
		unsigned _size;
		contiguous<std::set<unsigned>*> _data;

		void remove_set(unsigned i)
		{
			delete _data[i];
			_data.erase(_data.begin()+i);
		}

		//void connected_components(unsigned** E);
		
	public:
		DisjointSet(unsigned size)
		: _size(size)
		{ }

		void make_set(unsigned v)
		{
			_data.push_back(new std::set<unsigned>());
			_data.back()->insert(v);
		}

		void make_union(unsigned Vi, unsigned Vj)
		{
			unsigned index = find_set_index(Vj);
			unsigned index2 = find_set_index(Vi);
			
			std::set<unsigned>& setJ = *_data[index];
			std::set<unsigned>& setI = *_data[index2];
			
			if (index2 == index)
				return;
			setI.insert(setJ.begin(), setJ.end());
			remove_set(index);
		}

		unsigned find_set_index(unsigned x) const
		{
			for (unsigned i = 0; i < _data.size(); i++)
				if (_data[i]->find(x) != _data[i]->end())
					return i;
			return _data.size();
		}

		std::set<unsigned> const& find_set(unsigned x) const
		{ return *_data[find_set_index(x)]; }

		std::set<unsigned>& find_set(unsigned x)
		{ return *_data[find_set_index(x)]; }

		bool same_component(unsigned u, unsigned v) const
		{
			const std::set<unsigned> set = find_set(u);
			return set.find(v) != set.end();
		}
	};

	template<template<class> class C, class T>
	std::pair<contiguous<unsigned>,double> dijkstra(MetaGraph<C,T> const& graph, unsigned from, unsigned to)
	{
		std::pair<contiguous<unsigned>,double> ret;
		ret.second = 0;

		unsigned count = 1;
		DisjointSet set(graph.vertices());
		contiguous<double> weights(graph.vertices(), 1);
		contiguous<unsigned> previous(graph.vertices(), -1);
		std::queue<unsigned> queue;

		weights[from] = 0;
		set.make_set(from);
		
		Node<C,T> v = graph.get(from);
		for (unsigned i = 0; i < v.edges.size(); i++)
			if (from != v.edges[i].index)
			{
				weights[v.edges[i].index] = v.edges[i].value.value;
				previous[v.edges[i].index] = from;
				queue.push(v.edges[i].index);
			}

		while (queue.size())
			if (!set.same_component(from,queue.front()))
			{
				unsigned e = queue.front();
				set.make_set(e);
				set.make_union(from, e);
				v = graph.get(e);
				queue.pop();
				for (unsigned i = 0; i < v.edges.size(); i++)
					if (!set.same_component(from,v.edges[i].index))
						if (weights[e] + v.edges[i].value.value < weights[v.edges[i].index])
						{
							weights[v.edges[i].index] = weights[e] + v.edges[i].value.value;
							previous[v.edges[i].index] = e;
							queue.push(v.edges[i].index);
						}
			} else
				queue.pop();

		ret.first.push_back(previous[to]);
		if (previous[to] != -1)
			while(ret.first.front() != from)
				ret.first.insert(ret.first.begin(),previous[ret.first.front()]);
		ret.second = weights[to];

		return ret;
	}
}

#endif