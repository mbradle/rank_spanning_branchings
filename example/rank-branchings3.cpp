//=======================================================================
// Copyright 2015 Clemson University
// Authors: Bradley S. Meyer
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
  Example execution:

    ./rank-branchings2 branching_input.txt -10

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>

#include "../include/rank_spanning_branchings.hpp"

// When rank_spanning_branchings finds a branching, it calls the user-defined 
// functor (in this case, my_function).  The functor takes as input
// a filtered view of the parent graph that gives the branching.  If the
// functor returns false, rank_spanning_branchings will stop ranking the
// branchings and will return.  If the functor returns true,
// rank_spanning_branchings will continue to find the next branching.

using namespace boost;

class Matrix
{
  public:
    Matrix(size_t rows, size_t cols) :
      mRows( rows ), mCols( cols ), mData( rows * cols )
      {
        for( size_t i = 0; i < mRows; i++ )
        {
           for( size_t j = 0; j < mCols; j++ )
           {
              mData[i * mCols + j] = 0;
           }
        }
      }

    void update( size_t i, size_t j, double d )
    {
      mData[i * mCols + j] += d;
    }

    double get( size_t i, size_t j )
    {
      return mData[i * mCols + j];
    }

    size_t get_num_rows() { return mRows; }

    size_t get_num_cols() { return mCols; }

  private:
    size_t mRows;
    size_t mCols;
    std::vector<double> mData;
};

class dfs_my_visitor : public default_dfs_visitor
{
  public:
    dfs_my_visitor( ){}
    dfs_my_visitor( Matrix& matrix, double w ) :  m_w( w )
    {
      p_matrix = &matrix;
    }

    template<typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g) const
    {
      typename property_map<Graph, vertex_name_t>::const_type name_map;
      typename property_map<Graph, vertex_index_t>::const_type index_map;
      if( name_map[source( e, g )] == "root" )
      {
        i = index_map[target( e, g )];
      }
      j = index_map[target( e, g )];
      (*p_matrix).update( i, j, m_w );
    }
  private:
    mutable size_t i, j;
    Matrix* p_matrix = NULL;
    double m_w;
};

template<class Graph>
class my_function
{

  public:

    typedef
      typename property_map<Graph, edge_weight_t>::const_type WeightMap;

    typedef
      typename property_traits<WeightMap>::value_type weight_t;

    my_function( weight_t& _max_weight, weight_t _cut, std::vector<Matrix>& v_Matrix ) :
      max_weight( _max_weight ), cut( _cut ), m_v_Matrix( v_Matrix )
    {}

    template<class BranchingGraph>
    bool operator()( BranchingGraph& bg )
    {

      WeightMap w;

      typename graph_traits<BranchingGraph>::vertex_descriptor root;

      typename property_map<BranchingGraph, vertex_name_t>::const_type name_map;

      weight_t weight = 0;

      BGL_FORALL_EDGES_T( e, bg, BranchingGraph )
      {
        weight += get( w, e );
      }

      if( max_weight == -std::numeric_limits<weight_t>::infinity() )
      {
        max_weight = weight;
      }

      d_diff = weight - max_weight;

      if( d_diff < cut )
      {
        return false;
      }  // Stop before output.

      BGL_FORALL_VERTICES_T( v, bg, BranchingGraph )
      {
        if( name_map[v] == "root" ) { root = v; }
      }

      size_t n = num_vertices( bg );

      Matrix matrix( n, n );

      dfs_my_visitor vis( matrix, exp( d_diff ) );
      depth_first_search( bg, visitor( vis ).root_vertex( root ) );

      m_v_Matrix.push_back( matrix );

      return true;

    }

  private:
    weight_t& max_weight;
    weight_t cut;
    weight_t d_diff;
    std::vector<Matrix>& m_v_Matrix;

};
   
// Check for vertex from input file and add if not already present.

template <typename Graph, typename Vertex>
void
check_vertex( Graph& g, std::map<std::string, Vertex>& s_map, std::string str )
{

  typedef typename property_map < Graph, vertex_index_t >::type index_map_t;
  typedef typename property_map < Graph, vertex_name_t >::type name_map_t;

  index_map_t index_map = get( vertex_index, g );
  name_map_t name_map = get( vertex_name, g );

  if( s_map.find( str ) == s_map.end() )
  {
    Vertex u = add_vertex( g );
    index_map[u] = s_map.size();
    put( name_map, u, str );
    s_map[str] = u;
  }

}

// Read in the graph from the file.  Each line in the input file should be
// a comma-delimited triplet (source, target, weight).

template <typename Graph>
void
read_graph_file(
  std::istream & graph_in,
  Graph & g
)
{

  typedef typename property_map<Graph, edge_weight_t>::const_type WeightMap;

  typedef typename property_traits<WeightMap>::value_type weight_t;

  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  std::map<std::string, Vertex> s_map;
  std::string text;

  char_separator<char> sep(",");

  while( std::getline( graph_in, text ) )
  {

    std::vector<std::string> str;
    tokenizer<char_separator<char> > tokens( text, sep );

    BOOST_FOREACH( const std::string& name, tokens )
    {
      std::string s = name;
      algorithm::trim( s );
      str.push_back( s );
    }

    check_vertex( g, s_map, str[0] );  // Check source;
    check_vertex( g, s_map, str[1] );  // Check target;

    add_edge(
      s_map[str[0]], s_map[str[1]], lexical_cast<weight_t>( str[2] ), g
    );

  }

}

int main( int argc, char **argv )
{

  typedef float my_type;

  typedef adjacency_list <
      listS, listS, directedS,
      property <
        vertex_index_t, size_t,
        property< vertex_name_t, std::string >
      >,
      property < edge_weight_t, my_type>
    > Graph;
  std::ifstream input_file;
  my_type max_weight = -std::numeric_limits<my_type>::infinity();

  typedef typename property_map < Graph, vertex_index_t >::type index_map_t;
  typedef typename property_map < Graph, vertex_name_t >::type name_map_t;

  if( argc != 3 )
  {
    std::cerr << std::endl;
    std::cerr << "  Usage: " << argv[0] <<
                 " file  cut" << std::endl << std::endl;
    std::cerr << "    file = input file" << std::endl;
    std::cerr << "    cut = branching weight cut" << std::endl << std::endl;
    std::cerr << "  Example usage: " << argv[0] <<
                 " branching_input.txt -10" << std::endl << std::endl;;
    return EXIT_FAILURE;
  }

  Graph g;

  input_file.open( argv[1] );

  if( !input_file.is_open() )
  {
    std::cerr << "Invalid input file." << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream file_in( argv[1] );

  read_graph_file( file_in, g );

  index_map_t index_map = get( vertex_index, g );
  name_map_t name_map = get( vertex_name, g );

  std::map<size_t, std::string> r_map;

  BGL_FORALL_VERTICES_T( v, g, Graph )
  {
    r_map[index_map[v]] = name_map[v];
  }

  std::vector<Matrix> v_Matrix;

  rsb::rank_spanning_branchings(
    g,
    my_function<Graph>(
      max_weight,
      lexical_cast<my_type>( argv[2] ),
      v_Matrix
    )
  );

  for( size_t n = 0; n < v_Matrix.size(); n++ )
  {
    for( size_t i = 0; i < v_Matrix[n].get_num_rows(); i++ )
    {
       for( size_t j = 0; j < v_Matrix[n].get_num_cols(); j++ )
       {
         std::cout << r_map[i] << "  " << r_map[j] << "  " <<
           v_Matrix[n].get( i, j ) << std::endl;
//         std::cout << i << "  " << j << "  " << v_Matrix[n].get( i, j ) << std::endl;
       }
    }
    std::cout << std::endl;
  }

  return EXIT_SUCCESS;

}
