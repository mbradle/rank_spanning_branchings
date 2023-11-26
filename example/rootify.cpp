#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "../include/rank_spanning_branchings.hpp"

typedef double my_type;

using namespace boost;

enum edge_sign_t
{       
  edge_sign
};      
BOOST_INSTALL_PROPERTY( edge, sign );

template<class Graph>
class my_function
{

  public:

    typedef
      typename property_map<Graph, edge_weight_t>::const_type WeightMap;

    typedef
      typename property_traits<WeightMap>::value_type weight_t;

    typedef
      typename property_map<Graph, edge_sign_t>::const_type SignMap;

    typedef
      typename property_traits<SignMap>::value_type sign_t;

    my_function( weight_t& _max_weight, sign_t& _max_sign ) :
      max_weight( _max_weight ), max_sign( _max_sign ) { }

    template<class BranchingGraph>
    bool operator()( BranchingGraph& bg )
    {

      WeightMap w;
      SignMap s;

      BGL_FORALL_EDGES_T( e, bg, BranchingGraph )
      {
        max_weight += get( w, e );
        max_sign *= get( s, e );
      }

      return false;

    }

  private:
    weight_t& max_weight;
    sign_t& max_sign;

};
   
// Read in the graph from the matrix file.

template <typename Graph>
void
read_file(
  std::istream & input_file,
  Graph & g
)
{

  typedef std::map<std::pair<int, int>, my_type> mat_map_t;

  typedef typename property_map<Graph, edge_weight_t>::const_type WeightMap;

  typedef typename property_traits<WeightMap>::value_type weight_t;

  typedef typename property_map<Graph, edge_sign_t>::const_type SignMap;

  typedef typename property_traits<SignMap>::value_type sign_t;

  typedef typename graph_traits<Graph>::edge_descriptor Edge;

  Edge e;
  bool b_added;

  size_t row, col;
  weight_t weight;
  mat_map_t mat_map;

  while( input_file >> row >> col >> weight )
  {
    mat_map[std::make_pair(row, col)] = weight;
  }

  size_t N = 0;

  BOOST_FOREACH( mat_map_t::value_type &t, mat_map )
  {
    if( t.first.first > N ) { N = t.first.first; }    
    if( t.first.second > N ) { N = t.first.second; }    
  }

  std::vector<my_type> v_diag(N+1);

  BOOST_FOREACH( mat_map_t::value_type &t, mat_map )
  {
    v_diag[t.first.second] += t.second;
  }

  BOOST_FOREACH( mat_map_t::value_type &t, mat_map )
  { 
    if( t.first.first != t.first.second )
    {
      weight = -t.second;
      sign_t sgn = 1;
      if( weight < 0 )
      {
        sgn = -1;
        weight *= -1;
      }
      weight = log( weight ); 
        
      tie( e, b_added ) = add_edge( t.first.first, t.first.second, weight, g );

      put( edge_sign_t(), g, e, sgn );
    }
  }

  for( size_t i = 1; i < v_diag.size(); i++ )
  {
    weight = v_diag[i];
    sign_t sgn = 1;
    if( weight < 0 )
    {
      sgn = -1;
      weight *= -1;
    }
    weight = log( weight ); 
        
    tie( e, b_added ) = add_edge( 0, i, weight, g );
    
    put( edge_sign_t(), g, e, sgn );
  }

}

template<class Graph, class T>
void scale_edges( Graph& g, T scale_weight )
{
  BGL_FORALL_EDGES_T( e, g, Graph )
  {
    put(
      edge_weight_t(),
      g,
      e,
      (
        exp( get( edge_weight_t(), g, e ) - scale_weight )
      ) * get( edge_sign_t(), g, e )
    );
  }
}

template<class Graph, class Vertex>
Vertex find_root( Graph& g )
{
  Vertex root;
  BGL_FORALL_VERTICES_T( v, g, Graph )
  {
    if( in_degree( v, g ) == 0 )
    {
      root = v;
    }
  }
  return root;
}

template<class Graph, class Vertex>
void rootify( Graph g, Vertex root, Vertex v, my_type& result )
{
  typedef
    typename property_map<Graph, edge_weight_t>::const_type WeightMap;

  WeightMap w;

  typedef
    typename property_traits<WeightMap>::value_type weight_t;

  typedef typename graph_traits<Graph>::edge_descriptor Edge;
  Edge ef;
  bool b_found;

  std::vector<Edge> v_edges_to_remove;

  BGL_FORALL_INEDGES_T( v, e, g, Graph )
  {
    if( source( e, g ) != root )
    {
      v_edges_to_remove.push_back( e );
    }
  }

  BOOST_FOREACH( Edge e, v_edges_to_remove )
  {
    remove_edge( e, g );
  }

  v_edges_to_remove.clear();
  BGL_FORALL_OUTEDGES_T( v, e, g, Graph )
  {
    tie( ef, b_found ) = edge( root, target( e, g ), g );
    if( b_found )
    {
      put( edge_weight_t(), g, ef, get( w, e ) + get( w, ef ) );
    }
    else
    {
      add_edge( root, target( e, g ), get( w, e ), g );
    }
    v_edges_to_remove.push_back( e );
  }
  BOOST_FOREACH( Edge e, v_edges_to_remove )
  {
    remove_edge( e, g );
  }

  std::vector<Vertex> v_r;
  BGL_FORALL_OUTEDGES_T( root, e, g, Graph )
  {
    if( out_degree( target( e, g ), g )  > 0 )
    {
      v_r.push_back( target( e, g ) );
    }
  }

  if( v_r.size() == 0 )
  {
    weight_t weight = 1;
    BGL_FORALL_EDGES_T( e, g, Graph )
    {
      weight *= get( w, e );
    }
    result += weight;
    return;
  }
  else
  {
    BOOST_FOREACH( Vertex u, v_r )
    {
      rootify<Graph, Vertex>( g, root, u, result );
      remove_edge( root, u, g );
    }
  }

} 

template<class Graph, class Vertex>
my_type compute_determinant_from_roots( Graph g )
{
  my_type result = 0;

  Vertex root;

  std::vector<Vertex> v_r;

  root = find_root<Graph, Vertex>( g );

  BGL_FORALL_OUTEDGES_T( root, e, g, Graph )
  {
    if( out_degree( target( e, g ), g )  > 0 )
    {
      v_r.push_back( target( e, g ) );
    }
  }

  BOOST_FOREACH( Vertex v, v_r )
  {
    rootify<Graph, Vertex>( g, root, v, result );
    remove_edge( root, v, g );
  }

  return result;

} 

int main( int argc, char **argv )
{

  typedef adjacency_list <
      vecS, vecS, bidirectionalS,
      property <
        vertex_index_t, size_t,
        property< vertex_name_t, std::string >
      >,
      property < edge_weight_t, my_type, property < edge_sign_t, int> >
    > Graph;
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;

  typedef typename property_map<Graph, edge_sign_t>::const_type SignMap;
  typedef typename property_traits<SignMap>::value_type sign_t;

  std::ifstream input_file;

  my_type max_weight = 0;
  sign_t max_sign = 1;

  if( argc != 2 )
  {
    std::cerr << std::endl;
    std::cerr << "  Usage: " << argv[0] <<
                 " file" << std::endl << std::endl;
    std::cerr << "    file = input file" << std::endl;
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

  read_file( file_in, g );

  rsb::rank_spanning_branchings(
    g,
    my_function<Graph>(
      max_weight,
      max_sign
    )
  );

  double scale =  max_weight / ( num_vertices( g ) - 1 );

  scale_edges( g, scale );

  my_type result = compute_determinant_from_roots<Graph, Vertex>( g );

  std::cout << exp( max_weight ) * max_sign * result << std::endl;

  return EXIT_SUCCESS;

}
