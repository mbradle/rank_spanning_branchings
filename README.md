# Rank Spanning Branchings

## Overview

*rank_spanning_branchings()* is a routine to compute, in order, the optimal branchings in a directed graph.  It is implemented as a C++ header file that can be included in other code.  Because it uses variadic templates and default template arguments for a function template, it requires [C++11](https://en.wikipedia.org/wiki/C%2B%2B11), or a later version of C++.

## Prototype

The prototype for *rank_spanning_branchings()* is

          template <template<class...> class PriorityQueue = boost::heap::fibonacci_heap,
            typename Graph, typename BranchingProcessor,
            typename P, typename T, typename R>
          void
          rank_spanning_branchings(Graph& g, BranchingProcessor bp, 
              const bgl_named_params<P, T, R>& params = all defaults);
              
### Template-Only Parameters

IN: *PriorityQueue*

>A priority queue for keeping branching weights and condensed graph arc weights during discovery of the optimal branchings.  The priority queue must model [Priority Queue](https://www.boost.org/doc/libs/1_75_0/doc/html/heap/concepts.html) and it must be [mergable](https://www.boost.org/doc/libs/1_75_0/doc/html/heap/concepts.html#heap.concepts.merge).  The default is the [Boost Fibonacci Heap](https://www.boost.org/doc/libs/1_75_0/doc/html/boost/heap/fibonacci_heap.html).
              
### Parameters

IN: *const Graph&amp; g*

>A directed graph. The graph type must be a model of [VertexAndEdgeListGraph](https://www.boost.org/doc/libs/1_75_0/libs/graph/doc/VertexAndEdgeListGraph.html) The graph should have a single root vertex, that is, a single vertex that has indegree equal to zero and an edge directed to at least one other vertex. No other vertex should have indegree equal to zero. A graph with at least one spanning branching can always be constructed from a general directed graph by adding a vertex (the root vertex) and then adding edges with weight zero directed from the root vertex to every other vertex.

IN: *BranchingProcessor bp*

>A functor that models BranchingProcessor to process the edges in a branching.

### Named Parameters

IN: *weight_map(WeightMap w_map)*

>The weight or "length" of each edge in the graph. The *WeightMap* type must be a model of [ReadablePropertyMap](https://www.boost.org/doc/libs/1_75_0/libs/property_map/doc/ReadablePropertyMap.html).  If no edge weight comparison function is supplied, its value type must be [Less Than Comparable](https://www.boost.org/sgi/stl/LessThanComparable.html). The key type of this map needs to be the graph's edge descriptor type. Default: *get(edge_weight, g)*.

IN: *vertex_index_map(VertexIndexMap i_map)*

>This maps each vertex to an integer in the range `[0,num_vertices(g))`. The type *VertexIndexMap* must be a model of [Readable Property Map](https://www.boost.org/doc/libs/1_75_0/libs/property_map/doc/ReadablePropertyMap.html). The value type of the map must be an integer type. The vertex descriptor type of the graph needs to be usable as the key type of the map.  Default: *get(vertex_index, g)*.  Note: if you use this default, make sure your graph has an internal *vertex_index* property. For example, *adjacenty_list* with *VertexList=listS* does not have an internal *vertex_index* property.

IN: *distance_compare(CompareFunction cmp)*

>This function is used to compare edge weights to determine which edges to include in the next best branching.  It is also used to compare the weights of branchings.  The weight of a branching is the sum of weights of its edges. The *CompareFunction* type must be a model of [Binary Predicate](https://www.boost.org/sgi/stl/BinaryPredicate.html) and have argument types that match the value type of the *WeightMap* property map. Default: *std::less&lt;W&gt;* with *W=typename property_traits&lt;WeightMap&gt;::value_type*.

## BranchingProcessor

BranchingProcessor is used in the *rank_spanning_branchings()* function to process the current branching.  It is a functor that is called with a [Boost Filtered Graph](https://www.boost.org/doc/libs/1_75_0/libs/graph/doc/filtered_graph.html) view of the parent graph.  The processor must return a boolean.  If the return value is true, *rank_spanning_branchings()* continues and seeks the next branching. If the return value is false, *rank_spanning_branchings()* stops.

The following functor is an example BranchingProcessor that prints out the edges in a branching:

    struct print_branching
    {

      print_branching() {}

      template&lt;class BranchingGraph&gt;
      bool operator()( const BranchingGraph& bg )
      {
        std::cout << "Branching:";
        BGL_FORALL_EDGES_T( e, bg, BranchingGraph )
        {
          std::cout << " (" << source( e, bg ) << "," << target( e, bg ) << ")";
        }
        std::cout << std::endl;
        return true;
      }
    };
    
It returns true, so *rank_spanning_branchings()* would always continue on to find the next branching.

## Complexity

The time complexity of the routine is *O(kE log V)*, where *k* is the number of spanning branchings found.

## Example

The example directory included with this distribution contains several example codes that show how to use *rank_spanning_branchings()*.


## Acknowledgments

The development of *rank_spanning_branchings()* was inspired by the [C++ implementation](http://edmonds-alg.sourceforge.net) of Edmond's algorithm by Ali Tofigh and Erik Sj&ouml;lund.


Copyright &copy; 2015-2021 Clemson University, [Bradley S. Meyer](mailto:mbradle@clemson.edu).
