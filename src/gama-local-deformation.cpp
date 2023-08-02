/*
    GNU Gama -- adjustment of geodetic networks
    Copyright (C) 2013  Ales Cepek <cepek@gnu.org>

    This file is part of the GNU Gama C++ library.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  $
*/

const char* gama_local_deformation_version = "0.10";

#include <iostream>
bool gama_local_deformation_help(std::ostream& out,int argc, char *argv[]);

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>

#include <gnu_gama/local/svg.h>
#include <gnu_gama/xml/localnetwork_adjustment_results.h>
using Results = GNU_gama::LocalNetworkAdjustmentResults;

#include <matvec/bandmat.h>

// #define DEBUG_GAMA_LOCAL_DEFORMATION

std::string argv_epoch1, argv_epoch2, argv_text_file, argv_svg_file;

int idw {1}, indw {1};

struct Rec {
    std::string id;
    int indx {0}; double dx {0};
    int indy {0}; double dy {0};
    int indz {0}; double dz {0};

    //int dim {0};
};

struct Rec2 {
    std::string id;
    int indx1 {0}; double x1 {0};
    int indy1 {0}; double y1 {0};
    int indz1 {0}; double z1 {0};
    int indx2 {0}; double x2 {0};
    int indy2 {0}; double y2 {0};
    int indz2 {0}; double z2 {0};

    bool empty() const { return dim() == 0; }

    int dim() const {
        int d = 0;
        if (indz1*indz2) d += 1;
        if (indx1*indx2) d += 2;
        return d;
    }

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    friend std::ostream& operator<<(std::ostream& os, const Rec2& rec);
#endif
};

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
std::ostream& operator<<(std::ostream& os, const Rec2& rec)
{
    using std::setw;
    os << "  " << rec.id << "  "
       << setw(3) << rec.indx1 << " "
       << setw(3) << rec.indy1 << " "
       << setw(3) << rec.indz1 << setw(3) << " "
       << rec.x1    << " " << rec.y1    << " " << rec.z1    << " \t"
       << setw(3) << rec.indx2 << " "
       << setw(3) << rec.indy2 << " "
       << setw(3) << rec.indz2 << "   "
       << rec.x2    << " " << rec.y2    << " " << rec.z2 << "\n"
        ;
    return os;
}
#endif


int main(int argc, char *argv[])
{
    if (gama_local_deformation_help(std::cerr, argc, argv)) return 1;

    std::map<std::string, Rec2> adjrec;

    const auto epoch1 = std::make_unique<Results>();
    {
        std::ifstream inp_epoch1(argv_epoch1);
        if (!inp_epoch1) {
            std::cout << "   ####  ERROR OPENING 1st EPOCH ADJUSTMENT FILE "
                      << argv_epoch2 << "\n";
            return 1;
        }
        epoch1->read_xml(inp_epoch1);
    }

    for (const auto& p1 : epoch1->adjusted_points)
    {
        auto& rec = adjrec[p1.id];
        rec.id = p1.id; // p1.cxy p1.cz constrained
        rec.indx1 = p1.indx; rec.x1 = p1.x;
        rec.indy1 = p1.indy; rec.y1 = p1.y;
        rec.indz1 = p1.indz; rec.z1 = p1.z;
    }

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    for (const auto& r : adjrec) std::cerr << r.second;
    std::cerr << epoch1->cov;
#endif


    auto epoch2 = std::make_unique<Results>();
    {
        std::ifstream inp_epoch2(argv_epoch2);
        if (!inp_epoch2) {
            std::cout << "   ####  ERROR OPENING 2nd EPOCH ADJUSTMENT FILE "
                      << argv_epoch2 << "\n";
            return 1;
        }
        epoch2->read_xml(inp_epoch2);
    }
    for (const auto& p2 : epoch2->adjusted_points)
    {
        auto& rec = adjrec[p2.id];
        rec.id = p2.id; // p1.cxy p1.cz constrained
        rec.indx2 = p2.indx; rec.x2 = p2.x;
        rec.indy2 = p2.indy; rec.y2 = p2.y;
        rec.indz2 = p2.indz; rec.z2 = p2.z;
    }

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    for (const auto& r : adjrec) std::cerr << r.second;
    std::cerr << epoch2->cov;
#endif


    int adjcov_dim = 0;
    for (const auto& r : adjrec) adjcov_dim += r.second.dim();


#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    // std::cerr << adjcov << "\n";
#endif

    std::vector<int> t1 {0}, t2 {0}; // 1 based index transformation
    for (const auto& r : adjrec)
    {
        if (r.second.indx1 && r.second.indx2) {
            t1.push_back( r.second.indx1 );
            t1.push_back( r.second.indy1 );

            t2.push_back( r.second.indx2 );
            t2.push_back( r.second.indy2 );
        }
        if (r.second.indz1 && r.second.indz2) {
            t1.push_back( r.second.indz1 );
            t2.push_back( r.second.indz2 );
        }
    }

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    std::cerr << "\nadjcov_dim = " << adjcov_dim << "\n";

    std::cerr << "t1 = ";
    for (int i=1; i<=adjcov_dim; i++) {
        std::cerr << t1[i] << " ";
    }
    std::cerr << std::endl;

    std::cerr << "t2 = ";
    for (int i=1; i<=adjcov_dim; i++) {
        std::cerr << t2[i] << " ";
    }
    std::cerr << "\n\n";
#endif

    idw = 0;
    for (auto& adjr : adjrec) {
        int k = adjr.second.id.length();
        idw  = std::max(k, idw);
    }

    int cov_index {0};
    std::map<std::string, Rec> adjdiff;
    for (auto& adjr : adjrec)
        if (!adjr.second.empty())
        {
            std::string id = adjr.second.id;
            Rec rec;
            rec.id  = id;
            rec.dx  = adjr.second.x2 - adjr.second.x1;
            rec.dy  = adjr.second.y2 - adjr.second.y1;
            rec.dz  = adjr.second.z2 - adjr.second.z1;
            // rec.dim = adjr.second.dim();

            if (adjr.second.dim() == 3) {
                rec.indx = ++cov_index;
                rec.indy = ++cov_index;
                rec.indz = ++cov_index;
            }
            else if (adjr.second.dim() == 2) {
                rec.indx = ++cov_index;
                rec.indy = ++cov_index;
            }
            else if (adjr.second.dim() == 1) {
                rec.indz = ++cov_index;
            }

            adjdiff[id] = rec;
        }

    std::cout <<R"""(
# gama-local-deformation
# ----------------------
#
# point id | covariance indexes | x,y,z shifts (epoch2-epoch1) | epoch2  x,y,z

)""";

    int prec  {5};
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    std::cout.precision(prec);

    int indxw {0}, indyw {0}, indzw {0};
    for (auto& rec : adjdiff)
    {
        auto& r = rec.second;
        {
            std::ostringstream strx;
            strx.precision(prec);
            strx.setf(std::ios_base::fixed);
            strx << r.dx;
            indxw = std::max<int>(indxw, strx.str().length());
        }{
            std::ostringstream stry;
            stry.precision(prec);
            stry.setf(std::ios_base::fixed);
            stry << r.dy;
            indyw = std::max<int>(indyw, stry.str().length());
        }{
            std::ostringstream strz;
            strz.precision(prec);
            strz.setf(std::ios_base::fixed);
            strz << r.dz;
            indzw = std::max<int>(indzw, strz.str().length());
        }
    }

    indw = 1 + std::log10<int>(cov_index);
    std::cout.precision(prec);

    for (auto& rec : adjdiff)
    {
        auto& r = rec.second;
        std::cout << std::setw(idw)   << r.id   << "   "

                  << std::setw(indw)  << r.indx << " "
                  << std::setw(indw)  << r.indy << " "
                  << std::setw(indw)  << r.indz << "   "

                  << std::setw(indxw) << r.dx   << "  "
                  << std::setw(indyw) << r.dy   << "  "
                  << std::setw(indzw) << r.dz   << "    "

                  << adjrec[r.id].x2 << "  "
                  << adjrec[r.id].y2 << "  "
                  << adjrec[r.id].z2

                  << std::endl;
    }

    std::cout << "\n\n# deformation covariance matrix of x,y,z shifts\n\n";

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    std::cerr << epoch1->cov << "\n\n" << epoch2->cov << "\n\n";
#endif

    GNU_gama::CovMat<> C(cov_index, cov_index-1);
    for (int i=1; i<=cov_index; i++)
        for (int j=i; j<=cov_index; j++)
        {
            C(i,j) = epoch1->cov(t1[i],t1[j]) + epoch2->cov(t2[i],t2[j]);
        }

    std::cout << C;


    if (argv_svg_file.empty()) return 0;

    auto IS = new GNU_gama::local::LocalNetwork;
    //------------------------------------------
    {
        std::string epoch_is = epoch2->network_general_parameters.epoch;
        if (!epoch_is.empty()) IS->set_epoch(std::stod(epoch_is.c_str()));

        std::string algorithm = epoch2->network_general_parameters.gama_local_algorithm;
        IS->set_algorithm(algorithm);

        std::string axes = epoch2->network_general_parameters.axes_xy;
        IS->PD.local_coordinate_system
            = GNU_gama::local::LocalCoordinateSystem::string2locos(axes);

        std::string angles = epoch2->network_general_parameters.angles;
        if (angles == "left-handed")
            IS->PD.setAngularObservations_Lefthanded();
        else if (angles == "right-handed")
            IS->PD.setAngularObservations_Righthanded();
        else
            ; // this should never happen, keep implicit setting (left-handed)

    }


    for (const auto& ptfix : epoch2->fixed_points)
    {
        if (!ptfix.hxy) continue;

        auto& point = IS->PD[ptfix.id];
        point = GNU_gama::local::LocalPoint(ptfix.x, ptfix.y, ptfix.z);
        point.set_fixed_xy();
        point.set_unused_z();

        point.index_x() = point.index_y() = point.index_z() = 0;
    }

    for (const auto& ptadj : epoch2->adjusted_points)
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

#ifdef DEBUG_GAMA_LOCAL_DEFORMATION
    std::cerr << "\n****** IS->export_xml()\n\n" << IS->export_xml();
#endif

    GNU_gama::local::StandPoint* standpoint = new GNU_gama::local::StandPoint(&IS->OD);
    for (const auto& obs : epoch2->obslist)
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

    for (auto& e : epoch2->ellipses)
    {
        IS->stash_std_error_ellipse(e.id, e.major, e.minor, e.alpha);
    }

    GNU_gama::local::set_gama_language(GNU_gama::local::en);

    GNU_gama::local::GamaLocalSVG svg(IS);
    std::ofstream svg_file(argv_svg_file);

    for (const auto& rec : adjdiff) {
        auto id = rec.first;
        int dim = rec.second.indx + rec.second.indy + rec.second.indz;

        double dx = rec.second.dx;
        double dy = rec.second.dy;
        double dz = rec.second.dz;

        svg.shifts[id] = std::make_tuple(dim, dx, dy, dz);
    }

    svg.draw(svg_file);
}


bool gama_local_deformation_help(std::ostream& out, int argc, char *argv[])
{
    auto help = R"(
https://www.gnu.org/software/gama/

Usage: gama-local-deformation epoch1.xml epoch2.xml [--text file] [--svg file]
       gama-local-deformation --version
       gama-local-deformation --help

Options:

epoch1 and epoch2 are adjustment results in XML format of the surveying network.
           The program computes the shift vectors of common adjusted points
           and their corresponding covariance matrix.

--text     deformation analyses in textual format. If missing, standard
           output device is used (i.e. screen).
--svg      if defined, the program writes SVG image of the second epoch
           adjustment with standard deviation ellipses and points' shits.
           The network schema is available only in 2D (xy coordinates only).
--version
--help


Report bugs to: <bug-gama@gnu.org>
GNU gama home page: <https://www.gnu.org/software/gama/>
General help using GNU software: <https://www.gnu.org/gethelp/>
)";

    auto print_help = [help](std::ostream& out) { out << help; return true; };

    static const std::map<std::string, std::string> equivalents
    {
        {"-help", "-h"}, {"--help", "-h"},
        {"-version", "-v"}, {"--version", "-v"},
        {"--svg", "-svg"},
        {"--text", "-text"}
    };

    std::vector<std::string> epoch;

    for (int i=1; i<argc; i++)
    {
        if (argv[i][0] == '-')
        {
            std::string argvi = argv[i];
            auto ptr_argvi = equivalents.find(argvi);
            if (ptr_argvi != equivalents.end()) argvi = ptr_argvi->second;

            if (argvi == "-h") {
                return print_help(out);
            }

            if (argvi == "-v") {
                out << gama_local_deformation_version << "\n";
                return true;
            }

            if (argvi == "-svg") {
                if (i+1 <= argc){
                    argv_svg_file = argv[++i];
                }
                else {
                    return print_help(out);
                }
            }

            if (argvi == "-text") {
                if (i+1 <= argc) {
                    argv_text_file = argv[++i];
                }
                else {
                    return print_help(out);
                }
            }
        }
        else {
            epoch.push_back(argv[i]);
        }
    }

    if (epoch.size() != 2) return print_help(out);
    argv_epoch1 = epoch[0];
    argv_epoch2 = epoch[1];

    return false;
}
