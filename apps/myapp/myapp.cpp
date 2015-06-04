#include <graphlab.hpp>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <utility> //for pair
#include <iostream>
#include <string>

using std::vector;
using std::pair;
using std::string;
using std::cout;
using std::cin;
using std::endl;
using namespace graphlab;


typedef graphlab::vertex_id_type color_type;

struct set_union_gather{
    vector<color_type> colors;

    set_union_gather& operator+=(const set_union_gather& other){
        for(graphlab::vertex_id_type othervid = 0; 
                othervid < other.colors.size(); othervid++){
            colors.push_back(othervid);
        }
        return *this;
    }

    //serialize to disk 
    void save(graphlab::oarchive& oarc) const{
        oarc << colors; 
    }

    //unserilize from disk to RAM
    void load(graphlab::iarchive& iarc) {
        iarc >> colors;
    }
};

typedef graphlab::empty edge_data_type;
typedef graphlab::distributed_graph<color_type, edge_data_type> graph_type;

class setDistance: public graphlab::ivertex_program<graph_type, 
    set_union_gather>, 
    public graphlab::IS_POD_TYPE{
        public:

            edge_dir_type gather_edges(icontext_type &context, 
                    vertex_type vertex) const{
                return graphlab::IN_EDGES;
            }

            double getDist(vertex_data_type &a, vertex_data_type &b ){
                return 0;
            }

            //this happens for each edge 
            //probably in order then ?
            gather_type gather(icontext_type &context, 
                    vertex_type &vertex,
                    edge_type &edge){
                set_union_gather gather;
                double dist = getDist(edge.source().data(), edge.target().data());
                gather.colors.push_back(dist);
                return gather;
            }

            void apply(icontext_type &context, vertex_type &vertex, 
                    const gather_type &neighbor){
            
            }
    };



//input file will need to specify number of points.
int main(int argc, char** argv) 
{
    graphlab::mpi_tools::init(argc, argv);
    graphlab::distributed_control dc;

    graphlab::command_line_options clopts("KNN algorithm");


    //have to have a dc for some graph
    graph_type graph(dc);
    //graphlab::omni_engine<setDistance>engineSetDistance(dc, graph, "sync", clopts); 
    graphlab::async_consistent_engine<setDistance>engineSetDistance(dc, graph, clopts); 
    string path = "/u/qqiu/PowerGraph/release/apps/myapp/input/full";
    string format = "adj";
    graph.load_format(path,format);
    graph.finalize();

    //engineSetDistance.signal_all(); //This marks all points as "active"
    //engineSetDistance.start();


    graphlab::mpi_tools::finalize();
    return EXIT_SUCCESS; //from stdlib.h
}
