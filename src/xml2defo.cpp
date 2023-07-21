#include <iostream>
#include <iomanip>
#include <memory>
#include <string>

#include <gnu_gama/local/language.h>
#include <gnu_gama/local/svg.h>
#include <gnu_gama/local/results/text/general_parameters.h>
#include <gnu_gama/local/network.h>     // GNU_gama::local::LocalNetwork;
#include <gnu_gama/xml/localnetwork_adjustment_results.h>
#include <gnu_gama/local/lcoords.h>

// #include <gnu_gama/local/network.h>
// #include <gnu_gama/xml/gkfparser.h>

using Results = GNU_gama::LocalNetworkAdjustmentResults;


int main(int argc, char* argv[])
{
    std::cout << "xml2defo :  "
              //<< argc << " " << argv[0] << " "
              << (argc>1 ? argv[1] : "") << "\n";


    GNU_gama::local::set_gama_language(GNU_gama::local::en);

    auto adjxml = std::make_unique<Results>();
    {
        std::ifstream inp_epoch1(argv[1]);
        if (!inp_epoch1) {
            std::cout << "   ####  ERROR ON OPENING 1st EPOCH ADJUSTMENT FILE "
                      << argv[1] << "\n";
            return 1;
        }
        adjxml->read_xml(inp_epoch1);
    }


    auto IS = new GNU_gama::local::LocalNetwork;
    {
        std::string epoch_is = adjxml->network_general_parameters.epoch;
        if (!epoch_is.empty()) IS->set_epoch(std::stod(epoch_is.c_str()));

        std::string algorithm = adjxml->network_general_parameters.gama_local_algorithm;
        IS->set_algorithm(algorithm);

        std::string axes = adjxml->network_general_parameters.axes_xy;
        IS->PD.local_coordinate_system
            = GNU_gama::local::LocalCoordinateSystem::string2locos(axes);

    }


    for (const auto& ptfix : adjxml->fixed_points)
    {
        if (!ptfix.hxy) continue;

        auto& point = IS->PD[ptfix.id];
        point = GNU_gama::local::LocalPoint(ptfix.x, ptfix.y, ptfix.z);
        point.set_fixed_xy();
        point.set_unused_z();

        point.index_x() = point.index_y() = point.index_z() = 0;
    }

    for (const auto& ptadj : adjxml->adjusted_points)
    {
        if (!ptadj.hxy || ptadj.indx==0 || ptadj.indy==0) continue;

        auto& point = IS->PD[ptadj.id];
        point = GNU_gama::local::LocalPoint(ptadj.x, ptadj.y, ptadj.z);
        if (ptadj.cxy) point.set_constrained_xy();
        else           point.set_free_xy();
        point.set_unused_z();

        point.index_x() = ptadj.indx;
        point.index_y() = ptadj.indy;
        point.index_z() = 0;
    }

    GNU_gama::local::StandPoint* standpoint = new GNU_gama::local::StandPoint(&IS->OD);
    for (const auto& obs : adjxml->obslist)
    {
        using GNU_gama::local::Distance;

        if (obs.xml_tag == "angle")
        {
            auto dist1 = new Distance(obs.from, obs.left,  1.0);
            auto dist2 = new Distance(obs.from, obs.right, 1.0);
            standpoint->observation_list.push_back(dist1);
            standpoint->observation_list.push_back(dist2);
        }
        else
        {
            auto dist = new Distance(obs.from, obs.to, 1.0);
            standpoint->observation_list.push_back(dist);
        }
    }

    int k =standpoint->size();
    standpoint->covariance_matrix.reset(k,0);
    for (int i=1; i<=k; i++) standpoint->covariance_matrix(i,i) = 1;


    IS->OD.clusters.push_back(standpoint);

    for (auto& e : adjxml->ellipses)
    {
        IS->stash_std_error_ellipse(e.id, e.major, e.minor, e.alpha);
    }

    GNU_gama::local::set_gama_language(GNU_gama::local::en);

    //GeneralParameters(IS, std::cout);
    //std::cout << IS->export_xml();

    GNU_gama::local::GamaLocalSVG svg(IS);
//#define COUT_
#ifdef  COUT_
    svg.draw(std::cout);
#else
    std::ofstream ofstr("/home/cepek/GNU/gama-deformation/demo.svg");
    svg.draw(ofstr);
#endif
}
