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

#include <gnu_gama/xml/localnetwork_adjustment_results.h>
using Results = GNU_gama::LocalNetworkAdjustmentResults;

#include <matvec/bandmat.h>

// #define DEBUG

std::string argv_epoch1, argv_epoch2, argv_text_file, argv_svg_file;
int         argv_epoch_count {0};

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

#ifdef DEBUG
    friend std::ostream& operator<<(std::ostream& os, const Rec2& rec);
#endif
};

#ifdef DEBUG
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
    /*{
        std::cerr << "\n"
                  << "epoch1 " << argv_epoch1      << "   "
                  << "epoch2 " << argv_epoch2      << "   "
                  << "text "   << argv_text_file   << "   "
                  << "svg "    << argv_svg_file    << "   "
                  << "count "  << argv_epoch_count << "\n";

        return 0;
    }*/

    //std::cout << "******  " <<  argv_svg_file << " " << argv_text_file << "\n\n";
    //return 1;

    std::map<std::string, Rec2> adjrec;

    auto epoch1 = std::make_unique<Results>();
    {
        std::ifstream inp_epoch1(argv[1]);
        if (!inp_epoch1) {
            std::cout << "   ####  ERROR ON OPENING 1st EPOCH ADJUSTMENT FILE "
                      << argv[1] << "\n";
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

#ifdef DEBUG
    for (const auto& r : adjrec) std::cout << r.second;
    std::cout << epoch1->cov;
#endif


    auto epoch2 = std::make_unique<Results>();
    {
        std::ifstream inp_epoch2(argv[2]);
        if (!inp_epoch2) {
            std::cout << "   ####  ERROR ON OPENING 2nd EPOCH ADJUSTMENT FILE "
                      << argv[2] << "\n";
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

#ifdef DEBUG
    for (const auto& r : adjrec) std::cout << r.second;
    std::cout << epoch2->cov;
#endif


    int adjcov_dim = 0;
    for (const auto& r : adjrec) adjcov_dim += r.second.dim();


#ifdef DEBUG

    // std::cout << adjcov << "\n";
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

#ifdef DEBUG
    std::cout << "\nadjcov_dim = " << adjcov_dim << "\n";

    std::cout << "t1 = ";
    for (int i=1; i<=adjcov_dim; i++) {
        std::cout << t1[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "t2 = ";
    for (int i=1; i<=adjcov_dim; i++) {
        std::cout << t2[i] << " ";
    }
    std::cout << "\n\n";
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

    std::cout << "# point id | covariance indexes |"
                 " x, y, z shifts (epoch2 - epoch1) |"
                 " epoch2  x, y, z\n\n";

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

    std::cout << "\n\n# deformation covariance matrix of x, y, z shifts\n\n";

#ifdef DEBUG
    std::cout << epoch1->cov << "\n\n" << epoch2->cov << "\n\n";
#endif

    GNU_gama::CovMat<> C(cov_index, cov_index-1);
    for (int i=1; i<=cov_index; i++)
        for (int j=i; j<=cov_index; j++)
        {
            C(i,j) = epoch1->cov(t1[i],t1[j]) + epoch2->cov(t2[i],t2[j]);
        }

    std::cout << C;
}


bool gama_local_deformation_help(std::ostream& out, int argc, char *argv[])
{
    auto help = R""""(
https://www.gnu.org/software/gama/

Usage: gama-local-deformation epoch1.xml epoch2.xml [--text file] [--svg file]
       gama-local-deformation --version
       gama-local-deformation --help

Options:

epoch1 and epoch2 are adjustment results in XML format of the surveying network.
           The program computes the shift vectors of common adjusted points
           and their corresponding covariance matrix.

--text     deformation analises in textual format. If missing, standard
           output device is used (i.e. screen).
--svg      if defined, the program writes SVG image of the second epoch
           adjustment with standard deviation ellipses and points' shits.
           The network schema is available only in 2D (xy coordinates only).
--version
--help


Report bugs to: <bug-gama@gnu.org>
GNU gama home page: <https://www.gnu.org/software/gama/>
General help using GNU software: <https://www.gnu.org/gethelp/>
)"""";

    //if (argc < 3 || argc > 7) return true;

    for (int i=1; i<argc; i++)
    {
        if (argv[i][0] == '-')
        {
            std::string argvi = argv[i];
            if (argvi == "-h" || argvi == "-help" || argvi == "--help")
            {
                out << help;
                return true;
            }

            if (argvi == "-v" || argvi == "-version" || argvi == "--version")
            {
                out << gama_local_deformation_version << "\n";
                return false;
            }

            if (argvi == "--svg" || argvi == "-svg")
            {
                if (i+1 <= argc)
                {
                    argv_svg_file = argv[++i];
                }
                else
                {
                    return true;
                }
            }

            if (argvi == "--text" || argvi == "-text")
            {
                if (i+1 <= argc)
                {
                    argv_text_file = argv[++i];
                }
                else
                {
                    return true;
                }
            }
        }
        else {
            if      (argv_epoch_count == 0) argv_epoch1 = argv[i];
            else if (argv_epoch_count == 1) argv_epoch2 = argv[i];
            else return true;

            argv_epoch_count++;
        }
    }

    return false;
}
